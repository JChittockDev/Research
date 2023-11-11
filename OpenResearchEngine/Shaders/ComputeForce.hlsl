struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<Vertex> skinnedVertexBuffer : register(t0);
RWStructuredBuffer<Vertex> previousSkinnedVertexBuffer : register(u0);
RWStructuredBuffer<float3> forceVertexBuffer : register(u1);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int vertexID = dispatchThreadID.x;
    float3 position = skinnedVertexBuffer[vertexID].position - previousSkinnedVertexBuffer[vertexID].position;
    previousSkinnedVertexBuffer[vertexID].position = skinnedVertexBuffer[vertexID].position;
    forceVertexBuffer[vertexID] = position;
}