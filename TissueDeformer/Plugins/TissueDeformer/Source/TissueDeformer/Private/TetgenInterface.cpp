#include "TetgenInterface.h"

#define searchDistance 9999999.99999

void TetgenInterface::ToTetgenIO(FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& hole)
{
    const FPositionVertexBuffer& positionBuffer = RenderData->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;
    const FRawStaticIndexBuffer16or32Interface& indexBuffer = *(RenderData->LODRenderData[0].MultiSizeIndexContainer.GetIndexBuffer());

    int vertexCount = RenderData->LODRenderData[0].GetNumVertices();
    int triangleCount = indexBuffer.Num() / 3;

    tetgenio::facet* f;
    tetgenio::polygon* p;

    _in.firstnumber = 1;
    _in.numberofpoints = vertexCount;
    _in.pointlist = new REAL[_in.numberofpoints * 3];

    int pointIndex = 0;
    for (int vertexListIndex = 0; vertexListIndex < vertexCount; vertexListIndex++)
    {
        const FVector3f& position = positionBuffer.VertexPosition(vertexListIndex);
        _in.pointlist[pointIndex] = position.X; pointIndex++;
        _in.pointlist[pointIndex] = position.Y; pointIndex++;
        _in.pointlist[pointIndex] = position.Z; pointIndex++;
    }

    _in.numberoffacets = triangleCount;
    _in.facetlist = new tetgenio::facet[_in.numberoffacets];

    for (int facetIndex = 0; facetIndex < triangleCount; facetIndex++)
    {
        f = &_in.facetlist[facetIndex];
        f->numberofpolygons = 1;
        f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
        p = &f->polygonlist[0];
        p->numberofvertices = 3;
        p->vertexlist = new int[p->numberofvertices];
        f->numberofholes = 0;
        f->holelist = NULL;
        
        for (int facetVertexIndex = 0; facetVertexIndex < p->numberofvertices; facetVertexIndex++)
        {
            p->vertexlist[facetVertexIndex] = indexBuffer.Get(facetIndex * 3 + facetVertexIndex) + 1;
        }
    }

    _in.numberofholes = hole.Num();
    _in.holelist = new double[3 * _in.numberofholes];

    for (int holeID = 0, nHoles = hole.Num(); holeID < nHoles; holeID++)
    {
        _in.holelist[holeID * 3 + 0] = hole[holeID][0];
        _in.holelist[holeID * 3 + 1] = hole[holeID][1];
        _in.holelist[holeID * 3 + 2] = hole[holeID][2];
    }
}

// Output Tetgen mesh to maya object
void TetgenInterface::OutTetMeshData(TMap<const int, FVector>& inputVertexPositions, TMap<const int, FVector>& tetVertexPositions, 
    TMap<const int, int>& tetrahedralIDs, TMap<const int, TTuple<int, int>> edgeIDs, TMap<const int, TTuple<int, int, int>> triangleIDs, 
    int& vertexCount, int& tetCount, int& edgeCount, int& triangleCount, TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates)
{
    // Get output tet vertex data
    vertexCount = _out.numberofpoints;
    REAL* tetpoints = _out.pointlist;

    for (int i = 0; i < vertexCount * 3; i++)
    {
        const double x = tetpoints[i]; i++;
        const double y = tetpoints[i]; i++;
        const double z = tetpoints[i];
        const FVector point(x, y, z);
        tetVertexPositions[i] = point;
    }

    const int inputCount =  _in.numberofpoints;
    for (int i = 0; i < vertexCount * 3; i++)
    {
        const double x = tetpoints[i]; i++;
        const double y = tetpoints[i]; i++;
        const double z = tetpoints[i];
        const FVector point(x, y, z);
        inputVertexPositions[i] = point;
    }

    // Get output tet face data
    tetCount = _out.numberoftetrahedra * 4;

    for (int i = 0; i < tetCount; i++)
    {
        tetrahedralIDs[i] = _out.tetrahedronlist[i] - 1;
    }

    edgeCount = _out.numberofedges;
    int* edges = _out.edgelist;

    for (int i = 0; i < edgeCount/2; i++)
    {
        edgeIDs[i] = MakeTuple(edges[i * 2], edges[i * 2 + 1]);
    }

    triangleCount = _out.numberoftrifaces;
    int* tris = _out.trifacelist;
    
    for (int i = 0; i < triangleCount/3; i++)
    {
        triangleIDs[i] = MakeTuple(tris[i * 3] - 1, tris[i * 3 + 1] - 1, tris[i * 3 + 2] - 1);
    }

    GetClosestBaraycenter(inputVertexPositions, tetVertexPositions, triangleIDs, baraycentericCoordinates);
}

void TetgenInterface::ToTetMesh(double radius_edge_ratio, double volume_constraint, FSkeletalMeshRenderData* RenderData, 
    const TMap<const int, const FVector>& holes, TMap<const int, FVector>& inputVertexPositions, TMap<const int, FVector>& tetVertexPositions, 
    TMap<const int, int>& tetrahedralIDs, TMap<const int, TTuple<int, int>>& edgeIDs, TMap<const int, TTuple<int, int, int>>& triangleIDs, 
    int& vertexCount, int& tetCount, int& edgeCount, int& triangleCount, TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates)
{
    // Create Tetgen compatible data structure
    ToTetgenIO(RenderData, holes);
    // Convert this to a string command
    std::ostringstream strs;
    strs << radius_edge_ratio;
    std::string radius_edge_ratio_string = strs.str();
    strs.clear();
    strs.seekp(0);
    strs << volume_constraint;
    std::string volume_constraint_string = strs.str();
    std::string cmd = "pq" + radius_edge_ratio_string + "a" + volume_constraint_string;
    char* c_cmd = new char[cmd.size() + 1];
    std::copy(cmd.begin(), cmd.end(), c_cmd);
    c_cmd[cmd.size()] = '\0';
    // Perform terahedralization
    tetrahedralize(reinterpret_cast<tetgenbehavior*>(c_cmd), &_in, &_out);
    // Output as a Maya object
    OutTetMeshData(inputVertexPositions, tetVertexPositions, tetrahedralIDs, edgeIDs, triangleIDs,
                        vertexCount, tetCount, edgeCount, triangleCount, baraycentericCoordinates);
    delete[] c_cmd;
}

int TetgenInterface::FindClosestIndex(const FVector inputPoint, const TMap<const int, FVector>& targetPoints)
{
    float closestDistance = searchDistance;
    int closestIndex = 0;
    for (int y = 0; y < targetPoints.Num(); ++y)
    {
        float distance = FVector::Distance(inputPoint, targetPoints[y]);
        if (distance <= closestDistance)
        {
            closestDistance = distance;
            closestIndex = y;
        }
    }
    return closestIndex;
}

TMap<const int, int> TetgenInterface::GetClosestVertexIDs(const TMap<const int, FVector>& inputPoints, const TMap<const int, int>& includeIDs, const TMap<const int, FVector>& targetPoints)
{
    TMap<const int, int> targetIDs;
    for (int x = 0; x < includeIDs.Num(); ++x)
    {
        const FVector inputPoint = inputPoints[includeIDs[x]];
        targetIDs[x] = FindClosestIndex(inputPoint, targetPoints);
    }
    return targetIDs;
}

void TetgenInterface::GetBarycentricCoord(FVector inputPoint, FVector a, FVector b, FVector c, float& u, float& v, float& w)
{
    FVector v0 = b - a, v1 = c - a, v2 = inputPoint - a;
    float d00 = FVector::DotProduct(v0, v0);
    float d01 = FVector::DotProduct(v0, v1);
    float d11 = FVector::DotProduct(v1, v1);
    float d20 = FVector::DotProduct(v2, v0);
    float d21 = FVector::DotProduct(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

TArray<int> TetgenInterface::GetClosestTriangleID(int targetID, FVector position, TMap<const int, TTuple<int, int, int>>& triangleIDs, const TMap<const int, FVector>& points)
{
    float closestDistance = searchDistance;
    int closestIndex = 0;
    for (int x = 0; x < triangleIDs.Num(); ++x)
    {
        TTuple<int, int, int>* tri = &triangleIDs[x];

        const int id0 = tri->Get<0>();
        const int id1 = tri->Get<1>();
        const int id2 = tri->Get<2>();

        if (id0 == targetID || id1 == targetID || id2 == targetID)
        {
            const FVector averagePosition = (points[id0] + points[id1] + points[id2]) / 3.0;
            float distance = FVector::Distance(position, averagePosition);
            if (distance <= closestDistance)
            {
                closestDistance = distance;
                closestIndex = x;
            }
        }
    }
    return { closestIndex, targetID };
}

TMap<const int, int> TetgenInterface::InitializeIDMap(const int& size)
{
    TMap<const int, int> map;
    for (int i = 0; i < size; ++i)
    {
        map[i] = i;
    }
    return map;
}

void TetgenInterface::GetClosestBaraycenter(const TMap<const int, FVector>& originalPoints, 
    const TMap<const int, FVector>& tetPoints, TMap<const int, TTuple<int, int, int>>& triangleIDs, TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates)
{
    TMap<const int, int> includeIDs = InitializeIDMap(originalPoints.Num());
    TMap<const int, int> closestVertexIDs = GetClosestVertexIDs(originalPoints, includeIDs, tetPoints);

    for (int i = 0; i < originalPoints.Num(); ++i)
    {
        float u, v, w;
        int test = closestVertexIDs[i];
        TArray<int> triangleIndexData = GetClosestTriangleID(closestVertexIDs[i], originalPoints[i], triangleIDs, tetPoints);
        TTuple<int, int, int>* triangleID = &triangleIDs[triangleIndexData[0]];
        const int vertex1 = triangleID->Get<0>();
        const int vertex2 = triangleID->Get<1>();
        const int vertex3 = triangleID->Get<2>();
        GetBarycentricCoord(originalPoints[i], tetPoints[vertex1], tetPoints[vertex2], tetPoints[vertex3], u, v, w);
        TTuple<float, float, int, int> coordinate = MakeTuple(triangleIndexData[0], triangleIndexData[1], u, v);
        baraycentericCoordinates[i] = coordinate;
    }
}

void TetgenInterface::GenerateTetMesh(double radius_edge_ratio, double volume_constraint,
    FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& holes)
{
    ToTetMesh(radius_edge_ratio, volume_constraint, RenderData, holes, _inputVertexPositions,
        _tetVertexPositions, _tetrahedralIDs, _edgeIDs, _triangleIDs, _vertexCount, _tetCount, 
        _edgeCount, _triangleCount, _baraycentericCoordinates);
}