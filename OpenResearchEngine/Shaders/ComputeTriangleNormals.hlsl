struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct TangentNormals
{
    float3 normal;
    float3 tangent;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);
RWStructuredBuffer<TangentNormals> transformedNormalBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint triangleID = dispatchThreadID.x;
    uint indexVertex1 = indexBuffer[triangleID * 3];
    uint indexVertex2 = indexBuffer[triangleID * 3 + 1];
    uint indexVertex3 = indexBuffer[triangleID * 3 + 2];
    
    float3 vertex1 = inputVertexBuffer[indexVertex1].position;
    float3 vertex2 = inputVertexBuffer[indexVertex2].position;
    float3 vertex3 = inputVertexBuffer[indexVertex3].position;
    
    float2 uv1 = inputVertexBuffer[indexVertex1].texCoord;
    float2 uv2 = inputVertexBuffer[indexVertex2].texCoord;
    float2 uv3 = inputVertexBuffer[indexVertex3].texCoord;
    
    float3 edgeA = vertex2 - vertex1;
    float3 edgeB = vertex3 - vertex1;
    
    float2 uvEdgeA = uv2 - uv1;
    float2 uvEdgeB = uv3 - uv1;
    float r = 1.0 / (uvEdgeA.x * uvEdgeB.y - uvEdgeB.x * uvEdgeA.y);
    
    transformedNormalBuffer[triangleID].normal = normalize(cross(edgeA, edgeB));
    transformedNormalBuffer[triangleID].tangent = normalize(r * (edgeA * uvEdgeB.y - edgeB * uvEdgeA.y));
}