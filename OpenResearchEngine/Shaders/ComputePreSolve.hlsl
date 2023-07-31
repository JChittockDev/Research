struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<Vertex> simMeshSkinnedVertexBuffer : register(t0);
StructuredBuffer<uint3> simMeshPreviousSolverTransformBuffer : register(t1);
RWStructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(u0);

float3 UnQuantize(uint3 input, float factor)
{
    float vertexPositionX = ((float) input.x) / factor;
    float vertexPositionY = ((float) input.y) / factor;
    float vertexPositionZ = ((float) input.z) / factor;
    return float3(vertexPositionX, vertexPositionY, vertexPositionZ);
}

#define QUANTIZE 32768.0

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint simMeshVertexID = dispatchThreadID.x;
    float3 simMeshSkinnedVertexPosition = simMeshSkinnedVertexBuffer[simMeshVertexID].position;
    float3 simMeshPreviousSolverTransform = UnQuantize(simMeshPreviousSolverTransformBuffer[simMeshVertexID], QUANTIZE);
    
    float3 totalForces = simMeshSkinnedVertexPosition;
    
    simMeshTransformedVertexBuffer[simMeshVertexID].position = totalForces + simMeshPreviousSolverTransform;
}