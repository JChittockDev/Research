// Copyright Joseph Chittock @2022

#include "node.h"

UVSpringRelaxNode::UVSpringRelaxNode()
    : cachedRestEdgeCount(-1)
{
}

UVSpringRelaxNode::~UVSpringRelaxNode()
{
}

// ---------------------------------------------------------------------------
// Output geometry helper
// ---------------------------------------------------------------------------

MStatus UVSpringRelaxNode::setOutputGeom(const MPlug& plug, MDataBlock& block,
    MDataHandle& inputGeomHandle,
    const MFloatArray* newU, const MFloatArray* newV)
{
    MStatus status;

    MDataHandle outputGeomHandle = block.outputValue(aOutputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    outputGeomHandle.copy(inputGeomHandle);

    if (newU && newV)
    {
        MObject outputGeom = outputGeomHandle.asMesh();
        MFnMesh meshFn(outputGeom, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        status = meshFn.setUVs(*newU, *newV);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    block.setClean(plug);
    return MS::kSuccess;
}

// ---------------------------------------------------------------------------
// Compute
// ---------------------------------------------------------------------------

MStatus UVSpringRelaxNode::compute(const MPlug& plug, MDataBlock& block)
{
    if (plug != aOutputGeom)
        return MS::kUnknownParameter;

    MStatus status;

    const bool  enable     = block.inputValue(aEnableRelax,    &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const float stiffness  = block.inputValue(aStiffness,      &status).asFloat(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const float stepSize   = block.inputValue(aStepSize,       &status).asFloat(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  adaptive   = block.inputValue(aAdaptiveStepSize,&status).asBool(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const int   iterations = block.inputValue(aIterations,     &status).asInt();   CHECK_MSTATUS_AND_RETURN_IT(status);
    MString     uvSetName  = block.inputValue(aUVSet,          &status).asString(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  lockBorder = block.inputValue(aLockBorderUVs,  &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  relaxAxisU = block.inputValue(aRelaxAxisU,     &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  relaxAxisV = block.inputValue(aRelaxAxisV,     &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  jacobiDamp = block.inputValue(aJacobiDamping,  &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);

    MDataHandle inputGeomHandle = block.inputValue(aInputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject inputGeom = inputGeomHandle.asMesh();
    if (inputGeom.isNull())
        return MS::kSuccess;

    MDataHandle restGeomHandle = block.inputValue(aRestGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject restGeom = restGeomHandle.asMesh();

    if (restGeom.isNull() || !enable)
        return setOutputGeom(plug, block, inputGeomHandle, nullptr, nullptr);

    MFnMesh meshFn(inputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnMesh restMeshFn(restGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (uvSetName.length() == 0)
    {
        uvSetName = meshFn.currentUVSetName(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // Invalidate cache when rest topology or UV set changes
    const int edgeCount = restMeshFn.numEdges();
    if (!restEdgeLengths.empty() &&
        (uvSetName != cachedUVSetName || edgeCount != cachedRestEdgeCount))
    {
        restEdgeLengths.clear();
    }

    if (restEdgeLengths.empty())
    {
        restEdgeLengths.reserve(edgeCount);
        computeRestData(restMeshFn, restGeom, uvSetName, restEdgeLengths);
    }

    cachedUVSetName     = uvSetName;
    cachedRestEdgeCount = edgeCount;

    MPointArray points;
    status = meshFn.getPoints(points, MSpace::kObject);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFloatArray uArray, vArray;
    status = meshFn.getUVs(uArray, vArray, &uvSetName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    std::set<int> borderUVs;
    if (lockBorder)
        findUVShellBorders(inputGeom, uvSetName, borderUVs);

    MFloatArray newU, newV;
    if (!relaxUVs(meshFn, uArray, vArray, points, borderUVs,
                  stiffness, stepSize, iterations, newU, newV, adaptive, jacobiDamp))
        return MS::kFailure;

    if (!relaxAxisU) newU = uArray;
    if (!relaxAxisV) newV = vArray;

    return setOutputGeom(plug, block, inputGeomHandle, &newU, &newV);
}

// ---------------------------------------------------------------------------
// Rest data computation
// ---------------------------------------------------------------------------

void UVSpringRelaxNode::computeRestData(MFnMesh& meshFn, MObject& meshObj,
    const MString& uvSetName, std::vector<EdgeData>& outEdges)
{
    MStatus status;

    MPointArray restPoints;
    if (!meshFn.getPoints(restPoints, MSpace::kObject)) return;

    MFloatArray restU, restV;
    if (!meshFn.getUVs(restU, restV, &uvSetName)) return;

    MItMeshEdge edgeIter(meshObj, &status);
    if (!status) return;

    for (; !edgeIter.isDone(); edgeIter.next())
    {
        const int v0 = edgeIter.index(0, &status); if (!status) continue;
        const int v1 = edgeIter.index(1, &status); if (!status) continue;

        const float restLength3D = (float)(restPoints[v1] - restPoints[v0]).length();

        // Per-face UV lookup guarantees same-shell UV indices for both endpoints
        int   edgeUV0      = -1, edgeUV1 = -1;
        float restLengthUV = 0.0f;

        MIntArray connectedFaces;
        edgeIter.getConnectedFaces(connectedFaces);
        for (unsigned int fi = 0; fi < connectedFaces.length(); ++fi)
        {
            const int polyId = connectedFaces[fi];
            MIntArray faceVerts;
            if (!meshFn.getPolygonVertices(polyId, faceVerts)) continue;

            int li0 = -1, li1 = -1;
            for (unsigned int lv = 0; lv < faceVerts.length(); ++lv)
            {
                if (faceVerts[lv] == v0) li0 = (int)lv;
                if (faceVerts[lv] == v1) li1 = (int)lv;
            }
            if (li0 < 0 || li1 < 0) continue;

            int uvId0 = 0, uvId1 = 0;
            if (!meshFn.getPolygonUVid(polyId, li0, uvId0, &uvSetName)) continue;
            if (!meshFn.getPolygonUVid(polyId, li1, uvId1, &uvSetName)) continue;

            const float du = restU[uvId1] - restU[uvId0];
            const float dv = restV[uvId1] - restV[uvId0];
            restLengthUV = sqrtf(du * du + dv * dv);
            edgeUV0 = uvId0;
            edgeUV1 = uvId1;
            break;
        }

        outEdges.push_back({ restLength3D, restLengthUV, v0, v1, edgeUV0, edgeUV1 });
    }
}

// ---------------------------------------------------------------------------
// Border UV detection
// ---------------------------------------------------------------------------

void UVSpringRelaxNode::findUVShellBorders(MObject& meshObj,
    const MString& uvSetName, std::set<int>& borderUVs)
{
    MStatus status;
    std::map<int, std::set<int>> vertexUVs;

    MItMeshPolygon faceIter(meshObj, &status);
    if (!status) return;

    for (; !faceIter.isDone(); faceIter.next())
    {
        MIntArray verts;
        if (!faceIter.getVertices(verts)) continue;
        for (unsigned int i = 0; i < verts.length(); ++i)
        {
            int uvId = 0;
            if (faceIter.getUVIndex(i, uvId, &uvSetName))
                vertexUVs[verts[i]].insert(uvId);
        }
    }

    // Seam vertices: same mesh vertex maps to multiple UV indices
    for (const auto& kv : vertexUVs)
    {
        if (kv.second.size() > 1)
            borderUVs.insert(kv.second.begin(), kv.second.end());
    }

    // Mesh boundary edges: lock all UV indices on open borders
    MItMeshEdge edgeIter(meshObj, &status);
    if (!status) return;
    for (; !edgeIter.isDone(); edgeIter.next())
    {
        if (!edgeIter.onBoundary(&status)) continue;
        const int v0 = edgeIter.index(0, &status);
        const int v1 = edgeIter.index(1, &status);
        for (int v : { v0, v1 })
        {
            auto it = vertexUVs.find(v);
            if (it != vertexUVs.end())
                borderUVs.insert(it->second.begin(), it->second.end());
        }
    }
}

// ---------------------------------------------------------------------------
// Spring relaxation
// ---------------------------------------------------------------------------

bool UVSpringRelaxNode::relaxUVs(MFnMesh& meshFn,
    const MFloatArray& uArray, const MFloatArray& vArray,
    const MPointArray& currentPoints,
    const std::set<int>& borderUVs,
    float stiffness, float stepSize, int iterations,
    MFloatArray& newU, MFloatArray& newV,
    bool adaptiveStep, bool jacobiDamping)
{
    const unsigned int numUVs = uArray.length();
    if (numUVs == 0) return false;

    // Build O(1) border lookup to avoid repeated set searches in the hot loop
    std::vector<bool> isBorder(numUVs, false);
    for (int idx : borderUVs)
        if ((unsigned int)idx < numUVs) isBorder[idx] = true;

    newU = uArray;
    newV = vArray;

    std::vector<float> forceU(numUVs);
    std::vector<float> forceV(numUVs);
    std::vector<int>   nForce(numUVs);

    for (int iter = 0; iter < iterations; ++iter)
    {
        std::fill(forceU.begin(), forceU.end(), 0.0f);
        std::fill(forceV.begin(), forceV.end(), 0.0f);
        std::fill(nForce.begin(), nForce.end(), 0);

        float step = stepSize * stiffness;
        if (adaptiveStep)
            step *= (1.0f - (float)iter / (float)iterations * 0.5f);

        for (const EdgeData& data : restEdgeLengths)
        {
            if (data.uv0 < 0 || data.uv1 < 0) continue;

            const int  uv0 = data.uv0, uv1 = data.uv1;
            const bool b0  = isBorder[uv0];
            const bool b1  = isBorder[uv1];
            if (b0 && b1) continue;

            const float currentLength3D = (float)(currentPoints[data.v1] - currentPoints[data.v0]).length();
            const float stretchRatio3D  = (data.length > 0.0001f) ? currentLength3D / data.length : 1.0f;

            const float du    = newU[uv1] - newU[uv0];
            const float dv    = newV[uv1] - newV[uv0];
            const float lenUV = sqrtf(du * du + dv * dv);
            if (lenUV < 0.0001f) continue;

            const float stretchRatioUV = (data.uvLength > 0.0001f) ? lenUV / data.uvLength : 1.0f;
            if (fabsf(stretchRatio3D - stretchRatioUV) <= 0.001f) continue;

            const float edgeU        = du / lenUV;
            const float edgeV        = dv / lenUV;
            const float lengthChange = data.uvLength * stretchRatio3D - lenUV;

            const float share = 0.5f;
            const float dU    = edgeU * lengthChange * share * step;
            const float dV    = edgeV * lengthChange * share * step;

            if (!b0) { forceU[uv0] -= dU; forceV[uv0] -= dV; ++nForce[uv0]; }
            if (!b1) { forceU[uv1] += dU; forceV[uv1] += dV; ++nForce[uv1]; }
        }

        const float jacobiFactor = jacobiDamping ? 1.0f / (1.0f + step) : 1.0f;
        for (unsigned int i = 0; i < numUVs; ++i)
        {
            if (!isBorder[i] && nForce[i] > 0)
            {
                newU[i] += forceU[i] / (float)nForce[i] * jacobiFactor;
                newV[i] += forceV[i] / (float)nForce[i] * jacobiFactor;
            }
        }
    }
    return true;
}
