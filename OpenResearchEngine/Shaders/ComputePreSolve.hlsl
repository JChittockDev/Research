struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<float3> forceBuffer : register(t0);
StructuredBuffer<Vertex> inputVertexBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int vertexID = dispatchThreadID.x;
    float3 position = inputVertexBuffer[vertexID].position;
    outputVertexBuffer[vertexID].position = position + forceBuffer[vertexID];
}