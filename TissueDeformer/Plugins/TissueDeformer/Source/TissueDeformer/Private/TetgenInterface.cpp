#include "TetgenInterface.h"

#define searchDistance 9999999.99999

void TetgenInterface::ToTetgenIO(const TMap<const int, const FVector>& hole, tetgenio& in, 
    const TMap<const int, FVector>& inputVertexPositions, const TMap<const int, TTuple<int, int, int>>& inputTriangleIDs, 
    const TMap<const int, FVectorTangent>& inputTangents, const TArray<TMap<const int, FVector2D>>& inputTextureCoordinates)
{

    in.firstnumber = 1;
    in.numberofpoints = inputVertexPositions.Num();
    in.pointlist = new REAL[in.numberofpoints * 3];
    in.pointparamlist = new tetgenio::pointparam[in.numberofpoints];

    for (int vertexListIndex = 0; vertexListIndex < in.numberofpoints; vertexListIndex++)
    {
        in.pointlist[vertexListIndex * 3] = inputVertexPositions[vertexListIndex].X;
        in.pointlist[vertexListIndex * 3 + 1] = inputVertexPositions[vertexListIndex].Y;
        in.pointlist[vertexListIndex * 3 + 2] = inputVertexPositions[vertexListIndex].Z;

        tetgenio::pointparam param;
        param.tag = 0;
        param.type = 0;
        
        for (int t = 0; t < MAX_TEXCOORDS; ++t)
        {
            param.uv[t * 2 + 0] = inputTextureCoordinates[t][vertexListIndex].X;
            param.uv[t * 2 + 1] = inputTextureCoordinates[t][vertexListIndex].Y;
        }

        param.uv[MAX_TEXCOORDS * 2 + 0] = *((double*)(&inputTangents[vertexListIndex].TangentX.Vector.Packed));
        param.uv[MAX_TEXCOORDS * 2 + 1] = *((double*)(&inputTangents[vertexListIndex].TangentZ.Vector.Packed));
        param.uv[MAX_TEXCOORDS * 2 + 2] = 0;
        param.uv[MAX_TEXCOORDS * 2 + 3] = 0;
        param.uv[MAX_TEXCOORDS * 2 + 4] = 0;
        param.uv[MAX_TEXCOORDS * 2 + 5] = 0;
        in.pointparamlist[vertexListIndex] = param;
    }

    in.numberoffacets = inputTriangleIDs.Num();
    in.facetlist = new tetgenio::facet[in.numberoffacets];
    in.facetmarkerlist = new int[in.numberoffacets];

    
    for (int ti = 0; ti < in.numberoffacets; ti++)
    {
        in.facetmarkerlist[ti] = 1;

        tetgenio::facet& f = in.facetlist[ti];
        f.holelist = NULL;
        f.numberofholes = 0;
        f.numberofpolygons = 1;
        f.polygonlist = new tetgenio::polygon[f.numberofpolygons];

        tetgenio::polygon& p = f.polygonlist[0];
        p.numberofvertices = 3;
        p.vertexlist = new int[p.numberofvertices];
        p.vertexlist[0] = inputTriangleIDs[ti].Get<0>();
        p.vertexlist[1] = inputTriangleIDs[ti].Get<1>();
        p.vertexlist[2] = inputTriangleIDs[ti].Get<2>();
    }

    in.facetmarkerlist = NULL;

    int nholes = hole.Num();
    in.numberofholes = nholes;
    in.holelist = new double[in.numberofholes * 3];

    for (int holeid = 0; holeid < nholes; holeid++)
    {
        in.holelist[holeid * 3] = hole[holeid][0];
        in.holelist[holeid * 3 + 1] = hole[holeid][1];
        in.holelist[holeid * 3 + 2] = hole[holeid][2];
    }
}

// Output Tetgen mesh to maya object
void TetgenInterface::OutTetMeshData(TMap<const int, FVector>& inputVertexPositions, 
    TMap<const int, FVector>& tetVertexPositions, TMap<const int, int>& tetIDs, 
    TMap<const int, TTuple<int, int>> tetEdgeIDs, TMap<const int, TTuple<int, int, int>> tetTriangleIDs, 
    int& tetVertexCount, int& tetCount, int& tetEdgeCount, int& tetTriangleCount, 
    TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates, 
    const tetgenio& in, tetgenio& out)
{
    // Get output tet vertex data
    tetVertexCount = out.numberofpoints;
    REAL* tetpoints = out.pointlist;

    for (int i = 0; i < tetVertexCount; i++)
    {
        tetVertexPositions[i] = FVector(tetpoints[i * 3], tetpoints[i * 3 + 1], tetpoints[i * 3 + 2]);
    }

    // Get output tet face data
    tetCount = out.numberoftetrahedra * 4;

    for (int i = 0; i < tetCount; i++)
    {
        tetIDs[i] = out.tetrahedronlist[i] - 1;
    }

    tetEdgeCount = out.numberofedges;
    int* edges = out.edgelist;

    for (int i = 0; i < tetEdgeCount /2; i++)
    {
        tetEdgeIDs[i] = MakeTuple(edges[i * 2], edges[i * 2 + 1]);
    }

    tetTriangleCount = out.numberoftrifaces;
    int* tris = out.trifacelist;
    
    for (int i = 0; i < tetTriangleCount /3; i++)
    {
        tetTriangleIDs[i] = MakeTuple(tris[i * 3], tris[i * 3 + 1], tris[i * 3 + 2]);
    }

    GetClosestBaraycenter(inputVertexPositions, tetVertexPositions, tetTriangleIDs, baraycentericCoordinates);
}

tetgenbehavior behaviour(float radiusEdgeRatio, float volumeConstraint)
{
    tetgenbehavior b;
    {
        b.zeroindex = 1;
        b.docheck = 1;
        b.verbose = 1;
        b.diagnose = 0;
        b.facesout = 1;
        b.edgesout = 1;
        b.neighout = 2;
        b.object = tetgenbehavior::POLY;
        b.refine = 0;
        b.plc = 1;
        b.quality = 1;
        b.nomergefacet = 1;
        b.nomergevertex = 1;
        b.nojettison = 1;
        b.nobisect = 1;
        b.nobisect_nomerge = 1;
        b.supsteiner_level = 4;
        b.addsteiner_algo = 1;
        b.order = 1;
        b.psc = 1;
        b.minratio = radiusEdgeRatio;
        b.maxvolume = volumeConstraint;
    }
    return b;
}

void TetgenInterface::ToTetMesh(double radius_edge_ratio, double volume_constraint, FSkeletalMeshRenderData* RenderData, 
    const TMap<const int, const FVector>& holes, TMap<const int, FVector>& inputVertexPositions, 
    TMap<const int, TTuple<int, int, int>>& inputTriangleIDs, 
    TMap<const int, FVectorTangent>& inputTangents,
    TArray<TMap<const int, FVector2D>>& inputTextureCoordinates,
    TMap<const int, FVector>& tetVertexPositions, 
    TMap<const int, int>& tetIDs, TMap<const int, TTuple<int, int>>& tetEdgeIDs, 
    TMap<const int, TTuple<int, int, int>>& tetTriangleIDs, int& tetVertexCount, 
    int& tetCount, int& tetEdgeCount, int& tetTriangleCount, 
    TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates, 
    tetgenio& in, tetgenio& out)
{
    ToTetgenIO(holes, in, inputVertexPositions, inputTriangleIDs, inputTangents, inputTextureCoordinates);
    tetgenbehavior bhvr = behaviour(1.5, 1.5);
     //Perform terahedralization
    tetrahedralize(&bhvr, &in, &out);
    // Output as a Maya object
    OutTetMeshData(inputVertexPositions, tetVertexPositions, tetIDs, tetEdgeIDs, tetTriangleIDs,
        tetVertexCount, tetCount, tetEdgeCount, tetTriangleCount, baraycentericCoordinates, in, out);
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

void TetgenInterface::ParseMeshData(FSkeletalMeshRenderData* RenderData, TMap<const int, FVector>& InputVertexPositions, 
    TMap<const int, TTuple<int, int, int>>& InputTriangleIDs, TMap<const int, FVectorTangent>& inputTangents,
    TArray<TMap<const int, FVector2D>>& inputTextureCoordinates)
{
    const FPositionVertexBuffer& positionBuffer = RenderData->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;
    const FStaticMeshVertexBuffer& staticBuffer = RenderData->LODRenderData[0].StaticVertexBuffers.StaticMeshVertexBuffer;
    const FRawStaticIndexBuffer16or32Interface& indexBuffer = *(RenderData->LODRenderData[0].MultiSizeIndexContainer.GetIndexBuffer());

    int vertexCount = RenderData->LODRenderData[0].GetNumVertices();
    int triangleCount = indexBuffer.Num() / 3;

    for (int v = 0; v < vertexCount; v++)
    {
        const FVector3f& position = positionBuffer.VertexPosition(v);
        InputVertexPositions.Add(v, FVector(position.X, position.Y, position.Z));
    }

    for (int t = 0; t < triangleCount; t++)
    {
        InputTriangleIDs.Add(t, MakeTuple(indexBuffer.Get(t * 3),
                            indexBuffer.Get(t * 3 + 1),
                            indexBuffer.Get(t * 3 + 2)));
    }

    for (int tn = 0; tn < vertexCount; tn++)
    {
        auto tangentX = staticBuffer.VertexTangentX(tn);
        auto tangentZ = staticBuffer.VertexTangentZ(tn);

        FVectorTangent tangents;
        tangents.TangentX = FPackedRGB10A2N(tangentX);
        tangents.TangentZ = FPackedRGB10A2N(tangentZ);
        inputTangents.Add(tn, tangents);
    }

    for (int txc = 0; txc < MAX_TEXCOORDS; txc++)
    {
        TMap<const int, FVector2D> UVSet;
        inputTextureCoordinates.Add(UVSet);
    }

    for (int tx = 0; tx < vertexCount; tx++)
    {
        auto test = staticBuffer.GetVertexUV(tx, 0);
        for (int txc = 0; txc < MAX_TEXCOORDS; txc++)
        {
            inputTextureCoordinates[txc].Add(tx, FVector2D(test.X, test.Y));
        }
    }
}

void TetgenInterface::GenerateTetMesh(double radiusEdgeRatio, double volumeConstraint,
    FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& holes)
{

    ParseMeshData(RenderData, _inputVertexPositions, _inputTriangleIDs, _inputTangents, _inputTextureCoordinates);

    ToTetMesh(radiusEdgeRatio, volumeConstraint, RenderData, holes, _inputVertexPositions,
        _inputTriangleIDs, _inputTangents, _inputTextureCoordinates, _tetVertexPositions, _tetIDs, _tetEdgeIDs,
        _tetTriangleIDs, _tetVertexCount, _tetCount, _tetEdgeCount, _tetTriangleCount, _baraycentericCoordinates, 
        _in, _out);
}