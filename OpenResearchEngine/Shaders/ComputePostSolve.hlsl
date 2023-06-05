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
StructuredBuffer<InterlockedVector> simMeshTransformedVertexBuffer : register(t1);
RWStructuredBuffer<InterlockedVector> simMeshSpringTransformBuffer : register(u0);
RWStructuredBuffer<Vertex> simMeshPostSolveVertexBuffer : register(u1);

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
    
    float3 vertexPosition = UnQuantize(simMeshTransformedVertexBuffer[simMeshVertexID], factor);
    float3 offsetTransform = vertexPosition - simMeshSkinnedVertexBuffer[simMeshVertexID].position;
    
    int quantizedX = (int) (offsetTransform.x * factor);
    int quantizedY = (int) (offsetTransform.y * factor);
    int quantizedZ = (int) (offsetTransform.z * factor);
    int ignore;
 
    InterlockedExchange(simMeshSpringTransformBuffer[simMeshVertexID].x, quantizedX, ignore);
    InterlockedExchange(simMeshSpringTransformBuffer[simMeshVertexID].y, quantizedY, ignore);
    InterlockedExchange(simMeshSpringTransformBuffer[simMeshVertexID].z, quantizedZ, ignore);
    

    simMeshPostSolveVertexBuffer[simMeshVertexID].position = vertexPosition;
}
