struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct InterlockedVector
{
    int x;
    int y;
    int z;
};

StructuredBuffer<Vertex> simMeshSkinnedVertexBuffer : register(t0);
RWStructuredBuffer<Vertex> simMeshPreviousSkinnedVertexBuffer : register(u0);
RWStructuredBuffer<InterlockedVector> simMeshTransformedVertexBuffer : register(u1);

float3 UnQuantize(InterlockedVector input, float factor)
{
    float vertexPositionX = ((float) input.x) / factor;
    float vertexPositionY = ((float) input.y) / factor;
    float vertexPositionZ = ((float) input.z) / factor;
    return float3(vertexPositionX, vertexPositionY, vertexPositionZ);
}

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint simMeshVertexID = dispatchThreadID.x;
    float3 simSkinForce = simMeshSkinnedVertexBuffer[simMeshVertexID].position - simMeshPreviousSkinnedVertexBuffer[simMeshVertexID].position;

    float factor = 32768.0;
    int quantizedX = (int) (simSkinForce.x * factor);
    int quantizedY = (int) (simSkinForce.y * factor);
    int quantizedZ = (int) (simSkinForce.z * factor);
 
    InterlockedAdd(simMeshTransformedVertexBuffer[simMeshVertexID].x, quantizedX);
    InterlockedAdd(simMeshTransformedVertexBuffer[simMeshVertexID].y, quantizedY);
    InterlockedAdd(simMeshTransformedVertexBuffer[simMeshVertexID].z, quantizedZ);
    simMeshPreviousSkinnedVertexBuffer[simMeshVertexID] = simMeshSkinnedVertexBuffer[simMeshVertexID];
}