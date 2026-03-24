// Copyright Joseph Chittock @2022

#include "node.h"

UVSpringRelaxNode::UVSpringRelaxNode()
    : cachedRestEdgeCount(-1)
    , cachedRestHash(0.0f)
    , cachedInputEdgeCount(-1)
    , cachedLockMeshBoundary(true)
    , cachedLockUVSeams(true)
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
// Hash helper
// ---------------------------------------------------------------------------

float UVSpringRelaxNode::hashPoints(const MPointArray& pts)
{
    float h = 0.0f;
    const unsigned int n = pts.length();
    for (unsigned int i = 0; i < n; ++i)
        h += (float)(pts[i].x + pts[i].y * 31.0 + pts[i].z * 1009.0);
    return h;
}

// ---------------------------------------------------------------------------
// Compute
// ---------------------------------------------------------------------------

MStatus UVSpringRelaxNode::compute(const MPlug& plug, MDataBlock& block)
{
    if (plug != aOutputGeom)
        return MS::kUnknownParameter;

    MStatus status;

    const bool  enable       = block.inputValue(aEnableRelax,      &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const float stiffness    = block.inputValue(aStiffness,        &status).asFloat(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const float stepSize     = block.inputValue(aStepSize,         &status).asFloat(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  adaptive     = block.inputValue(aAdaptiveStepSize, &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const int   iterations   = block.inputValue(aIterations,       &status).asInt();   CHECK_MSTATUS_AND_RETURN_IT(status);
    MString     uvSetName    = block.inputValue(aUVSet,            &status).asString(); CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  lockBoundary = block.inputValue(aLockMeshBoundary, &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  lockSeams    = block.inputValue(aLockUVSeams,      &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  relaxAxisU   = block.inputValue(aRelaxAxisU,       &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  relaxAxisV   = block.inputValue(aRelaxAxisV,       &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const bool  jacobiDamp   = block.inputValue(aJacobiDamping,    &status).asBool();  CHECK_MSTATUS_AND_RETURN_IT(status);
    const float tolerance    = block.inputValue(aTolerance,        &status).asFloat(); CHECK_MSTATUS_AND_RETURN_IT(status);

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

    // Resolve UV set name, warn if specified set is missing
    if (uvSetName.length() == 0)
    {
        uvSetName = meshFn.currentUVSetName(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    else
    {
        MStringArray uvSetNames;
        status = meshFn.getUVSetNames(uvSetNames);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        bool found = false;
        for (unsigned int i = 0; i < uvSetNames.length(); ++i)
        {
            if (uvSetNames[i] == uvSetName) { found = true; break; }
        }
        if (!found)
        {
            MGlobal::displayWarning("UVSpringRelaxNode: UV set '" + uvSetName + "' not found, using current.");
            uvSetName = meshFn.currentUVSetName(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }
    }

    // Rest data cache — invalidate on topology, UV set, or position change
    const int restEdgeCount = restMeshFn.numEdges();
    MPointArray restPoints;
    restMeshFn.getPoints(restPoints, MSpace::kObject);
    const float restHash = hashPoints(restPoints);

    const bool restCacheInvalid = !restEdgeLengths.empty() &&
        (uvSetName != cachedUVSetName ||
         restEdgeCount != cachedRestEdgeCount ||
         restHash != cachedRestHash);
    if (restCacheInvalid)
        restEdgeLengths.clear();

    // Border cache — check before updating cachedUVSetName
    const int  inputEdgeCount   = meshFn.numEdges();
    const bool borderCacheInvalid = cachedBorderUVs.empty() ||
        inputEdgeCount != cachedInputEdgeCount ||
        uvSetName != cachedUVSetName ||
        lockBoundary != cachedLockMeshBoundary ||
        lockSeams != cachedLockUVSeams;

    if (restEdgeLengths.empty())
    {
        restEdgeLengths.reserve(restEdgeCount);
        computeRestData(restMeshFn, restGeom, uvSetName, restEdgeLengths);
    }

    cachedUVSetName     = uvSetName;
    cachedRestEdgeCount = restEdgeCount;
    cachedRestHash      = restHash;

    MPointArray points;
    status = meshFn.getPoints(points, MSpace::kObject);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFloatArray uArray, vArray;
    status = meshFn.getUVs(uArray, vArray, &uvSetName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (borderCacheInvalid)
    {
        cachedBorderUVs.clear();
        if (lockBoundary || lockSeams)
            findUVShellBorders(inputGeom, uvSetName, lockBoundary, lockSeams, cachedBorderUVs);
        cachedInputEdgeCount   = inputEdgeCount;
        cachedLockMeshBoundary = lockBoundary;
        cachedLockUVSeams      = lockSeams;
    }

    MFloatArray newU, newV;
    if (!relaxUVs(uArray, vArray, points, cachedBorderUVs,
                  stiffness, stepSize, iterations, newU, newV,
                  adaptive, jacobiDamp, tolerance))
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
    const MString& uvSetName,
    bool lockMeshBoundary, bool lockUVSeams,
    std::unordered_set<int>& borderUVs)
{
    MStatus status;
    std::unordered_map<int, std::unordered_set<int>> vertexUVs;

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

    if (lockUVSeams)
    {
        for (const auto& kv : vertexUVs)
        {
            if (kv.second.size() > 1)
                borderUVs.insert(kv.second.begin(), kv.second.end());
        }
    }

    if (lockMeshBoundary)
    {
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
}

// ---------------------------------------------------------------------------
// Spring relaxation
// ---------------------------------------------------------------------------

bool UVSpringRelaxNode::relaxUVs(
    const MFloatArray& uArray, const MFloatArray& vArray,
    const MPointArray& currentPoints,
    const std::unordered_set<int>& borderUVs,
    float stiffness, float stepSize, int iterations,
    MFloatArray& newU, MFloatArray& newV,
    bool adaptiveStep, bool jacobiDamping, float tolerance)
{
    const unsigned int numUVs = uArray.length();
    if (numUVs == 0) return false;

    std::vector<bool>  isBorder(numUVs, false);
    for (int idx : borderUVs)
        if ((unsigned int)idx < numUVs) isBorder[idx] = true;

    newU = uArray;
    newV = vArray;

    std::vector<float> forceU(numUVs);
    std::vector<float> forceV(numUVs);
    std::vector<float> nForce(numUVs);

    const float tolSq = tolerance * tolerance;

    for (int iter = 0; iter < iterations; ++iter)
    {
        std::fill(forceU.begin(), forceU.end(), 0.0f);
        std::fill(forceV.begin(), forceV.end(), 0.0f);
        std::fill(nForce.begin(), nForce.end(), 0.0f);

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
            if (fabsf(stretchRatio3D - stretchRatioUV) <= tolerance) continue;

            const float edgeU        = du / lenUV;
            const float edgeV        = dv / lenUV;
            const float lengthChange = data.uvLength * stretchRatio3D - lenUV;
            const float weight       = data.length;

            const float dU = edgeU * lengthChange * 0.5f * step;
            const float dV = edgeV * lengthChange * 0.5f * step;

            if (!b0) { forceU[uv0] -= dU * weight; forceV[uv0] -= dV * weight; nForce[uv0] += weight; }
            if (!b1) { forceU[uv1] += dU * weight; forceV[uv1] += dV * weight; nForce[uv1] += weight; }
        }

        const float jacobiFactor = jacobiDamping ? 1.0f / (1.0f + step) : 1.0f;

        float maxDispSq = 0.0f;
        for (unsigned int i = 0; i < numUVs; ++i)
        {
            if (!isBorder[i] && nForce[i] > 0.0f)
            {
                const float du = forceU[i] / nForce[i] * jacobiFactor;
                const float dv = forceV[i] / nForce[i] * jacobiFactor;
                newU[i] += du;
                newV[i] += dv;
                const float dispSq = du * du + dv * dv;
                if (dispSq > maxDispSq) maxDispSq = dispSq;
            }
        }

        if (maxDispSq < tolSq)
            break;
    }
    return true;
}
