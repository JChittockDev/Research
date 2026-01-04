struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct TriangleNeighbours
{
    uint index[8];
};

struct TangentNormals
{
    float3 normal;
    float3 tangent;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<TriangleNeighbours> triangleAdjacencyBuffer : register(t1);
StructuredBuffer<TangentNormals> transformedNormalBuffer : register(t2);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
    float3 vertexNormal = float3(0.0, 0.0, 0.0);
    float3 vertexTangent = float3(0.0, 0.0, 0.0);
    Vertex inputVertex = inputVertexBuffer[vertexID];
    
    for (int i = 0; i < 8; ++i)
    {
        uint neighbourTriangle = triangleAdjacencyBuffer[vertexID].index[i];
        
        if (neighbourTriangle != vertexID)
        {
            vertexNormal += transformedNormalBuffer[neighbourTriangle].normal;
            vertexTangent += transformedNormalBuffer[neighbourTriangle].tangent;

        }
    }
    
    Vertex outputVertex = inputVertex;
    outputVertex.normal = normalize(-vertexNormal);
    outputVertex.tangent = float4(normalize(-vertexTangent), 1.0);
    outputVertexBuffer[vertexID] = outputVertex;
}