struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<uint> transferIndexBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
    uint simMeshVertexID = transferIndexBuffer[vertexID];
    float3 simVertexPosition = inputVertexBuffer[simMeshVertexID].position;
    outputVertexBuffer[vertexID].position = simVertexPosition;
}