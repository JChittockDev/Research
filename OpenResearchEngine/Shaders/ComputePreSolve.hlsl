struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct Spring
{
    float3 transform;
};

StructuredBuffer<Vertex> simMeshSkinnedVertexBuffer : register(t0);
StructuredBuffer<Spring> simMeshSpringTransformBuffer : register(t1);
RWStructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint simMeshVertexID = dispatchThreadID.x;
    float3 simMeshSkinnedVertexPosition = simMeshSkinnedVertexBuffer[simMeshVertexID].position;
    
    simMeshTransformedVertexBuffer[simMeshVertexID].position = simMeshSkinnedVertexPosition + simMeshSpringTransformBuffer[simMeshVertexID].transform;
}