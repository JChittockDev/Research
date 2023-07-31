struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<Vertex> simMeshSkinnedVertexBuffer : register(t0);
RWStructuredBuffer<Vertex> simMeshOutputVertexBuffer : register(u0);
RWStructuredBuffer<uint3> simMeshPreviousSolverTransformBuffer : register(u1);
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
        float3 simMeshSkinnedTransform = simMeshSkinnedVertexBuffer[simMeshVertexID].position;
        float3 simMeshSolverTransform = UnQuantize(simMeshSolverTransformBuffer[simMeshVertexID], QUANTIZE) / (float) solverCount;
        float3 simMeshPreviousSolverTransform = UnQuantize(simMeshPreviousSolverTransformBuffer[simMeshVertexID], QUANTIZE);
        float3 simMeshOutputTransform = simMeshSkinnedTransform + simMeshPreviousSolverTransform + simMeshSolverTransform;
        
        simMeshOutputVertexBuffer[simMeshVertexID].position = simMeshOutputTransform;
        
        float3 reconstructionTransform = simMeshOutputTransform - simMeshSkinnedTransform;
        uint quantizedX = (uint) (reconstructionTransform.x * QUANTIZE);
        uint quantizedY = (uint) (reconstructionTransform.y * QUANTIZE);
        uint quantizedZ = (uint) (reconstructionTransform.z * QUANTIZE);
        
        //uint3 value = simMeshPreviousSolverTransformBuffer[simMeshVertexID];
        //InterlockedExchange(simMeshPreviousSolverTransformBuffer[simMeshVertexID].x, quantizedX, value.x);
        //InterlockedExchange(simMeshPreviousSolverTransformBuffer[simMeshVertexID].y, quantizedY, value.y);
        //InterlockedExchange(simMeshPreviousSolverTransformBuffer[simMeshVertexID].z, quantizedZ, value.z);
        
        simMeshPreviousSolverTransformBuffer[simMeshVertexID].x = quantizedX;
        simMeshPreviousSolverTransformBuffer[simMeshVertexID].y = quantizedY;
        simMeshPreviousSolverTransformBuffer[simMeshVertexID].z = quantizedZ;

    }
    
    simMeshSolverTransformBuffer[simMeshVertexID] = uint3(0, 0, 0);
    simMeshSolverCountBuffer[simMeshVertexID] = 0;
}
