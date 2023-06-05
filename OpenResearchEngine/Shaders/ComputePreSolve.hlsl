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

struct InterlockedVector
{
    int x;
    int y;
    int z;
};

StructuredBuffer<Vertex> simMeshSkinnedVertexBuffer : register(t0);
StructuredBuffer<InterlockedVector> simMeshSpringTransformBuffer : register(t1);
RWStructuredBuffer<InterlockedVector> simMeshTransformedVertexBuffer : register(u0);

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
    
    float factor = 32768.0;
    float3 simMeshPreSolveVertexPosition = simMeshSkinnedVertexBuffer[simMeshVertexID].position + UnQuantize(simMeshSpringTransformBuffer[simMeshVertexID], factor);

    int quantizedX = (int) (simMeshPreSolveVertexPosition.x * factor);
    int quantizedY = (int) (simMeshPreSolveVertexPosition.y * factor);
    int quantizedZ = (int) (simMeshPreSolveVertexPosition.z * factor);
    int ignore;
 
    InterlockedExchange(simMeshTransformedVertexBuffer[simMeshVertexID].x, quantizedX, ignore);
    InterlockedExchange(simMeshTransformedVertexBuffer[simMeshVertexID].y, quantizedY, ignore);
    InterlockedExchange(simMeshTransformedVertexBuffer[simMeshVertexID].z, quantizedZ, ignore);
    
}