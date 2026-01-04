// Copyright Joseph Chittock @2022

#include "node.h"

UVSpringRelaxNode::UVSpringRelaxNode()
{
}

UVSpringRelaxNode::~UVSpringRelaxNode()
{
}

MStatus UVSpringRelaxNode::setOutputGeom(const MPlug& plug, MDataBlock& block, const MObject& object, const MFloatArray* newU = nullptr, const MFloatArray* newV = nullptr)
{
    MStatus status;

    MDataHandle outputGeomHandle = block.outputValue(aOutputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Use the input mesh as output, just update UVs if provided
    MFnMesh meshFn(object, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (newU && newV)
    {
        status = meshFn.setUVs(*newU, *newV);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    outputGeomHandle.set(object);
    block.setClean(plug);

    return MS::kSuccess;
}

MStatus UVSpringRelaxNode::compute(const MPlug& plug, MDataBlock& block)
{
    if (plug != aOutputGeom)
        return MS::kUnknownParameter;

    MStatus status;

    // Get attribute values
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

    // Get the input geometry
    MDataHandle inputGeomHandle = block.inputValue(aInputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject inputGeom = inputGeomHandle.asMesh();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (inputGeom.isNull())
    {
        return MS::kSuccess;
    }

    // Get rest geometry from attribute
    MDataHandle restGeomHandle = block.inputValue(aRestGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MObject restGeom = restGeomHandle.asMesh();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Early exit if rest geometry is not set
    if (restGeom.isNull() || !enable)
    {
        // Pass through input geometry to output
        return setOutputGeom(plug, block, inputGeom);
    }

    // Create function sets
    MFnMesh meshFn(inputGeom, &status);
    MFnMesh restMeshFn(restGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Use default UV set if none specified
    if (uvSetName.length() == 0)
    {
        uvSetName = meshFn.currentUVSetName(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // Compute or retrieve rest edge lengths
    if (restEdgeLengths.size() == 0)
    {
        computeRestData(restMeshFn, restGeom, uvSetName, restEdgeLengths);
    }

    // Get current vertex positions
    MPointArray points;
    status = meshFn.getPoints(points, MSpace::kObject);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get current UVs
    MFloatArray uArray, vArray;
    status = meshFn.getUVs(uArray, vArray, &uvSetName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Build vertex to UV mapping
    std::map<int, int> vertToUV;
    buildVertexToUVMap(meshFn, uvSetName, vertToUV);

    // Find UV shell borders
    std::set<int> borderUVs;
    findUVShellBorders(meshFn, uvSetName, borderUVs);

    // Apply spring relaxation to UVs
    MFloatArray newU, newV;
    bool success = relaxUVs(meshFn, uArray, vArray, points, vertToUV, borderUVs, stiffness, stepSize, iterations, newU, newV, adaptiveStepSize);

    // Set the relaxed UVs back to the mesh
    if (success)
    {
        return setOutputGeom(plug, block, inputGeom, &newU, &newV);
    }
    else
    {
        return MS::kFailure;
    }
}


void UVSpringRelaxNode::computeRestData(MFnMesh& meshFn, MObject& meshObj, const MString& uvSetName, std::map<int, EdgeData>& restEdgeLengths)
{
    MStatus status;

    // Get rest positions
    MPointArray restPoints;
    status = meshFn.getPoints(restPoints, MSpace::kObject);
    if (!status)
        return;

    // Get rest UVs
    MFloatArray restU, restV;
    status = meshFn.getUVs(restU, restV, &uvSetName);
    if (!status)
        return;

    // Build vertex to UV mapping for rest mesh
    std::map<int, int> restVertToUV;
    int numPolygons = meshFn.numPolygons();
    for (int polyId = 0; polyId < numPolygons; ++polyId)
    {
        MIntArray vertexList;
        status = meshFn.getPolygonVertices(polyId, vertexList);
        if (!status) continue;

        for (unsigned int localVertId = 0; localVertId < vertexList.length(); ++localVertId)
        {
            int vertId = vertexList[localVertId];
            int uvId = 0;
            status = meshFn.getPolygonUVid(polyId, localVertId, uvId, &uvSetName);
            if (status)
            {
                restVertToUV[vertId] = uvId;
            }
        }
    }

    // Iterate through all edges
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

        // Calculate rest 3D length
        MPoint p0 = restPoints[v0];
        MPoint p1 = restPoints[v1];
        MVector diff = p1 - p0;
        float restLength3D = (float)diff.length();

        // Calculate rest UV length
        float restLengthUV = 0.0f;
        auto uvIt0 = restVertToUV.find(v0);
        auto uvIt1 = restVertToUV.find(v1);

        if (uvIt0 != restVertToUV.end() && uvIt1 != restVertToUV.end())
        {
            int uv0 = uvIt0->second;
            int uv1 = uvIt1->second;

            float u0 = restU[uv0];
            float v0 = restV[uv0];
            float u1 = restU[uv1];
            float v1 = restV[uv1];

            float du = u1 - u0;
            float dv = v1 - v0;
            restLengthUV = sqrtf(du * du + dv * dv);
        }

        EdgeData data;
        data.length = restLength3D;
        data.uvLength = restLengthUV;
        data.v0 = v0;
        data.v1 = v1;

        restEdgeLengths[edgeId] = data;
    }
}

void UVSpringRelaxNode::buildVertexToUVMap(MFnMesh& meshFn,
    const MString& uvSetName,
    std::map<int, int>& vertToUV)
{
    MStatus status;
    int numPolygons = meshFn.numPolygons();

    // For each polygon, get its vertices and their UV indices
    for (int polyId = 0; polyId < numPolygons; ++polyId)
    {
        MIntArray vertexList;
        status = meshFn.getPolygonVertices(polyId, vertexList);
        if (!status) continue;

        for (unsigned int localVertId = 0; localVertId < vertexList.length(); ++localVertId)
        {
            int vertId = vertexList[localVertId];
            int uvId = 0;
            status = meshFn.getPolygonUVid(polyId, localVertId, uvId, &uvSetName);
            if (status)
            {
                vertToUV[vertId] = uvId;
            }
        }
    }
}

void UVSpringRelaxNode::findUVShellBorders(MFnMesh& meshFn,
    const MString& uvSetName,
    std::set<int>& borderUVs)
{
    MStatus status;
    std::map<int, std::set<int>> vertexUVCounts;

    // Iterate through all faces to build vertex->UV mapping
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
            {
                vertexUVCounts[vertId].insert(uvId);
            }
        }
    }

    // Any vertex with multiple UV IDs indicates UV splits (borders)
    for (std::map<int, std::set<int>>::const_iterator it = vertexUVCounts.begin();
        it != vertexUVCounts.end(); ++it)
    {
        const std::set<int>& uvIds = it->second;
        if (uvIds.size() > 1)
        {
            // All UVs for this vertex are on borders
            for (std::set<int>::const_iterator uvIt = uvIds.begin();
                uvIt != uvIds.end(); ++uvIt)
            {
                borderUVs.insert(*uvIt);
            }
        }
    }

    // Also check for mesh boundary edges
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

            // Add all UVs associated with these vertices
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
    const std::map<int, int>& vertToUV,
    const std::set<int>& borderUVs,
    float stiffness,
    float stepSize,
    int iterations,
    MFloatArray& newU,
    MFloatArray& newV,
    const bool& adaptiveStep)
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
        {
			step *= (1.0f - (float)iter / iterations * 0.5f);
        }

        for (const auto& edge : restEdgeLengths)
        {
            const EdgeData& data = edge.second;

            // Get UV indices from vertex indices
            auto uvIt0 = vertToUV.find(data.v0);
            auto uvIt1 = vertToUV.find(data.v1);

            if (uvIt0 == vertToUV.end() || uvIt1 == vertToUV.end())
                continue;

            int uv0 = uvIt0->second;
            int uv1 = uvIt1->second;

            // Skip border UVs
            if (borderUVs.find(uv0) != borderUVs.end() ||
                borderUVs.find(uv1) != borderUVs.end())
                continue;

            // Calculate current 3D length
            MPoint p0 = currentPoints[data.v0];
            MPoint p1 = currentPoints[data.v1];
            float currentLength3D = (float)(p1 - p0).length();

            // Calculate 3D stretch ratio (what the deformation is)
            float stretchRatio3D = 1.0f;
            if (data.length > 0.0001f)
            {
                stretchRatio3D = currentLength3D / data.length;
            }

            // Get current UV positions
            float u0 = newU[uv0];
            float v0 = newV[uv0];
            float u1 = newU[uv1];
            float v1 = newV[uv1];

            // Calculate current UV length
            float du = u1 - u0;
            float dv = v1 - v0;
            float currentLengthUV = sqrtf(du * du + dv * dv);

            // Calculate UV stretch ratio (what the UVs currently are)
            float stretchRatioUV = 1.0f;
            if (data.uvLength > 0.0001f)
            {
                stretchRatioUV = currentLengthUV / data.uvLength;
            }

            // Calculate the mismatch between 3D deformation and UV stretch
            // If they match, stretchDifference = 0 and no correction is needed
            float stretchDifference = stretchRatio3D - stretchRatioUV;

            // If UVs don't match the 3D deformation, apply correction
            if (fabsf(stretchDifference) > 0.001f)
            {
                // Normalize the UV edge vector
                float edgeLength = sqrtf(du * du + dv * dv);
                if (edgeLength > 0.0001f)
                {
                    float edgeU = du / edgeLength;
                    float edgeV = dv / edgeLength;

                    // Calculate displacement needed to match 3D stretch
                    // Positive stretchDifference = UVs need to expand
                    // Negative stretchDifference = UVs need to contract
                    float targetLength = data.uvLength * stretchRatio3D;
                    float lengthChange = targetLength - currentLengthUV;

                    float displacementU = edgeU * lengthChange * 0.5f;
                    float displacementV = edgeV * lengthChange * 0.5f;

                    // Apply spring forces to correct the mismatch
                    forceU[uv0] -= displacementU * step;
                    forceV[uv0] -= displacementV * step;

                    forceU[uv1] += displacementU * step;
                    forceV[uv1] += displacementV * step;
                }
            }
        }

        // Apply forces
        for (unsigned int uvId = 0; uvId < numUVs; ++uvId)
        {
            newU[uvId] += forceU[uvId];
            newV[uvId] += forceV[uvId];
        }
    }

    return true;
}