// Copyright Joseph Chittock @2022

#include "node.h"

UVSpringRelaxNode::UVSpringRelaxNode()
    : cachedRestEdgeCount(-1)
{
}

UVSpringRelaxNode::~UVSpringRelaxNode()
{
}

MStatus UVSpringRelaxNode::setOutputGeom(const MPlug& plug, MDataBlock& block, MDataHandle& inputGeomHandle, const MFloatArray* newU, const MFloatArray* newV)
{
    MStatus status;

    MDataHandle outputGeomHandle = block.outputValue(aOutputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Copy input mesh data to output so we never mutate the upstream mesh
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

MStatus UVSpringRelaxNode::compute(const MPlug& plug, MDataBlock& block)
{
    if (plug != aOutputGeom)
        return MS::kUnknownParameter;

    MStatus status;

    MDataHandle enableHandle = block.inputValue(aEnableRelax, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    bool enable = enableHandle.asBool();

    MDataHandle stiffnessHandle = block.inputValue(aStiffness, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    float stiffness = stiffnessHandle.asFloat();

    MDataHandle stepSizeHandle = block.inputValue(aStepSize, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    float stepSize = stepSizeHandle.asFloat();

    MDataHandle adaptiveStepSizeHandle = block.inputValue(aAdaptiveStepSize, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    bool adaptiveStepSize = adaptiveStepSizeHandle.asBool();

    MDataHandle iterationsHandle = block.inputValue(aIterations, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    int iterations = iterationsHandle.asInt();

    MDataHandle uvSetHandle = block.inputValue(aUVSet, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MString uvSetName = uvSetHandle.asString();

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

    // Check each construction status independently
    MStatus meshStatus;
    MFnMesh meshFn(inputGeom, &meshStatus);
    CHECK_MSTATUS_AND_RETURN_IT(meshStatus);

    MStatus restMeshStatus;
    MFnMesh restMeshFn(restGeom, &restMeshStatus);
    CHECK_MSTATUS_AND_RETURN_IT(restMeshStatus);

    if (uvSetName.length() == 0)
    {
        uvSetName = meshFn.currentUVSetName(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // Invalidate rest data cache when rest mesh topology or UV set changes
    int currentRestEdgeCount = restMeshFn.numEdges();
    if (!restEdgeLengths.empty() &&
        (uvSetName != cachedUVSetName || currentRestEdgeCount != cachedRestEdgeCount))
    {
        restEdgeLengths.clear();
    }

    if (restEdgeLengths.empty())
    {
        computeRestData(restMeshFn, restGeom, uvSetName, restEdgeLengths);
        cachedUVSetName = uvSetName;
        cachedRestEdgeCount = currentRestEdgeCount;
    }

    MPointArray points;
    status = meshFn.getPoints(points, MSpace::kObject);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFloatArray uArray, vArray;
    status = meshFn.getUVs(uArray, vArray, &uvSetName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    std::set<int> borderUVs;
    findUVShellBorders(meshFn, uvSetName, borderUVs);

    MFloatArray newU, newV;
    bool success = relaxUVs(meshFn, uArray, vArray, points, borderUVs, stiffness, stepSize, iterations, newU, newV, adaptiveStepSize);

    if (success)
        return setOutputGeom(plug, block, inputGeomHandle, &newU, &newV);
    else
        return MS::kFailure;
}


void UVSpringRelaxNode::computeRestData(MFnMesh& meshFn, MObject& meshObj, const MString& uvSetName, std::map<int, EdgeData>& restEdgeLengths)
{
    MStatus status;

    MPointArray restPoints;
    status = meshFn.getPoints(restPoints, MSpace::kObject);
    if (!status)
        return;

    MFloatArray restU, restV;
    status = meshFn.getUVs(restU, restV, &uvSetName);
    if (!status)
        return;

    MItMeshEdge edgeIter(meshObj, &status);
    if (!status)
        return;

    for (; !edgeIter.isDone(); edgeIter.next())
    {
        int edgeId = edgeIter.index(&status);
        if (!status)
            continue;

        int v0 = edgeIter.index(0, &status);
        if (!status)
            continue;

        int v1 = edgeIter.index(1, &status);
        if (!status)
            continue;

        // 3D rest length
        MVector diff = restPoints[v1] - restPoints[v0];
        float restLength3D = (float)diff.length();

        // Per-face UV lookup: find an adjacent face and use getPolygonUVid to get UV
        // indices for both vertices in that face's context. This guarantees both indices
        // come from the same UV shell, which matters for seam-adjacent edges.
        int edgeUV0 = -1, edgeUV1 = -1;
        float restLengthUV = 0.0f;

        MIntArray connectedFaces;
        edgeIter.getConnectedFaces(connectedFaces);

        for (unsigned int fi = 0; fi < connectedFaces.length(); ++fi)
        {
            int polyId = connectedFaces[fi];

            MIntArray faceVerts;
            status = meshFn.getPolygonVertices(polyId, faceVerts);
            if (!status)
                continue;

            int localIdx0 = -1, localIdx1 = -1;
            for (unsigned int lv = 0; lv < faceVerts.length(); ++lv)
            {
                if (faceVerts[lv] == v0) localIdx0 = (int)lv;
                if (faceVerts[lv] == v1) localIdx1 = (int)lv;
            }

            if (localIdx0 < 0 || localIdx1 < 0)
                continue;

            int uvId0 = 0, uvId1 = 0;
            MStatus s0 = meshFn.getPolygonUVid(polyId, localIdx0, uvId0, &uvSetName);
            MStatus s1 = meshFn.getPolygonUVid(polyId, localIdx1, uvId1, &uvSetName);
            if (!s0 || !s1)
                continue;

            float du = restU[uvId1] - restU[uvId0];
            float dv = restV[uvId1] - restV[uvId0];
            restLengthUV = sqrtf(du * du + dv * dv);
            edgeUV0 = uvId0;
            edgeUV1 = uvId1;
            break;
        }

        EdgeData data;
        data.length   = restLength3D;
        data.uvLength = restLengthUV;
        data.v0       = v0;
        data.v1       = v1;
        data.uv0      = edgeUV0;
        data.uv1      = edgeUV1;

        restEdgeLengths[edgeId] = data;
    }
}

void UVSpringRelaxNode::findUVShellBorders(MFnMesh& meshFn,
    const MString& uvSetName,
    std::set<int>& borderUVs)
{
    MStatus status;
    std::map<int, std::set<int>> vertexUVCounts;

    MItMeshPolygon faceIter(meshFn.object(), &status);
    if (!status)
        return;

    for (; !faceIter.isDone(); faceIter.next())
    {
        MIntArray vertices;
        status = faceIter.getVertices(vertices);
        if (!status)
            continue;

        for (unsigned int i = 0; i < vertices.length(); ++i)
        {
            int vertId = vertices[i];
            int uvId = 0;
            status = faceIter.getUVIndex(i, uvId, &uvSetName);
            if (status)
                vertexUVCounts[vertId].insert(uvId);
        }
    }

    // Vertices with multiple UV indices are at UV seams — lock all their UVs
    for (std::map<int, std::set<int>>::const_iterator it = vertexUVCounts.begin();
        it != vertexUVCounts.end(); ++it)
    {
        const std::set<int>& uvIds = it->second;
        if (uvIds.size() > 1)
        {
            for (std::set<int>::const_iterator uvIt = uvIds.begin();
                uvIt != uvIds.end(); ++uvIt)
            {
                borderUVs.insert(*uvIt);
            }
        }
    }

    // Also lock UVs on mesh boundary edges
    MObject meshObj = meshFn.object();
    MItMeshEdge edgeIter(meshObj, &status);
    if (!status)
        return;

    for (; !edgeIter.isDone(); edgeIter.next())
    {
        if (edgeIter.onBoundary(&status))
        {
            int v0 = edgeIter.index(0, &status);
            int v1 = edgeIter.index(1, &status);

            if (vertexUVCounts.find(v0) != vertexUVCounts.end())
            {
                const std::set<int>& uvIds = vertexUVCounts[v0];
                borderUVs.insert(uvIds.begin(), uvIds.end());
            }
            if (vertexUVCounts.find(v1) != vertexUVCounts.end())
            {
                const std::set<int>& uvIds = vertexUVCounts[v1];
                borderUVs.insert(uvIds.begin(), uvIds.end());
            }
        }
    }
}

bool UVSpringRelaxNode::relaxUVs(MFnMesh& meshFn,
    const MFloatArray& uArray,
    const MFloatArray& vArray,
    const MPointArray& currentPoints,
    const std::set<int>& borderUVs,
    float stiffness,
    float stepSize,
    int iterations,
    MFloatArray& newU,
    MFloatArray& newV,
    bool adaptiveStep)
{
    unsigned int numUVs = uArray.length();
    if (numUVs == 0)
        return false;

    newU = uArray;
    newV = vArray;

    for (int iter = 0; iter < iterations; ++iter)
    {
        std::vector<float> forceU(numUVs, 0.0f);
        std::vector<float> forceV(numUVs, 0.0f);

        float step = stepSize * stiffness;
        if (adaptiveStep)
            step *= (1.0f - (float)iter / (float)iterations * 0.5f);

        for (const auto& edge : restEdgeLengths)
        {
            const EdgeData& data = edge.second;

            // UV indices were resolved per-face in computeRestData — correct shell guaranteed
            if (data.uv0 < 0 || data.uv1 < 0)
                continue;

            int uv0 = data.uv0;
            int uv1 = data.uv1;

            bool uv0IsBorder = borderUVs.count(uv0) > 0;
            bool uv1IsBorder = borderUVs.count(uv1) > 0;

            // No free endpoint — nothing to correct
            if (uv0IsBorder && uv1IsBorder)
                continue;

            // Current 3D stretch ratio
            MPoint p0 = currentPoints[data.v0];
            MPoint p1 = currentPoints[data.v1];
            float currentLength3D = (float)(p1 - p0).length();

            float stretchRatio3D = 1.0f;
            if (data.length > 0.0001f)
                stretchRatio3D = currentLength3D / data.length;

            // Current UV edge vector
            float u0 = newU[uv0], v0 = newV[uv0];
            float u1 = newU[uv1], v1 = newV[uv1];
            float du = u1 - u0;
            float dv = v1 - v0;
            float currentLengthUV = sqrtf(du * du + dv * dv);

            float stretchRatioUV = 1.0f;
            if (data.uvLength > 0.0001f)
                stretchRatioUV = currentLengthUV / data.uvLength;

            float stretchDifference = stretchRatio3D - stretchRatioUV;

            if (fabsf(stretchDifference) > 0.001f)
            {
                float edgeLength = sqrtf(du * du + dv * dv);
                if (edgeLength > 0.0001f)
                {
                    float edgeU = du / edgeLength;
                    float edgeV = dv / edgeLength;

                    float targetLength  = data.uvLength * stretchRatio3D;
                    float lengthChange  = targetLength - currentLengthUV;
                    float displacementU = edgeU * lengthChange * 0.5f;
                    float displacementV = edgeV * lengthChange * 0.5f;

                    // Only apply force to free (non-border) endpoints
                    if (!uv0IsBorder)
                    {
                        forceU[uv0] -= displacementU * step;
                        forceV[uv0] -= displacementV * step;
                    }
                    if (!uv1IsBorder)
                    {
                        forceU[uv1] += displacementU * step;
                        forceV[uv1] += displacementV * step;
                    }
                }
            }
        }

        // Apply forces — border UVs are never moved
        for (unsigned int uvId = 0; uvId < numUVs; ++uvId)
        {
            if (borderUVs.count(uvId) == 0)
            {
                newU[uvId] += forceU[uvId];
                newV[uvId] += forceV[uvId];
            }
        }
    }

    return true;
}
