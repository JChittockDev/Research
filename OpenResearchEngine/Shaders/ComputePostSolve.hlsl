struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

RWStructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(u0);
RWStructuredBuffer<Vertex> simMeshPreviousSolverTransformBuffer : register(u1);
RWStructuredBuffer<uint3> simMeshSolverTransformBuffer : register(u2);
RWStructuredBuffer<uint> simMeshSolverCountBuffer : register(u3);

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
    uint solverCount = simMeshSolverCountBuffer[simMeshVertexID];
    
    if (solverCount > 0)
    {
        float3 simMeshSolverTransform = UnQuantize(simMeshSolverTransformBuffer[simMeshVertexID], QUANTIZE) / solverCount;
        simMeshTransformedVertexBuffer[simMeshVertexID].position += simMeshSolverTransform;
        simMeshPreviousSolverTransformBuffer[simMeshVertexID].position = simMeshSolverTransform;
    }
    
    simMeshSolverTransformBuffer[simMeshVertexID] = uint3(0, 0, 0);
    simMeshSolverCountBuffer[simMeshVertexID] = 0;
}
