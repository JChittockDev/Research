struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct Edge
{
    uint vertexA;
    uint vertexB;
};

StructuredBuffer<Edge> inputEdgeBuffer : register(t0);
StructuredBuffer<float> inputEdgeConstraintBuffer : register(t1);
StructuredBuffer<Vertex> inputVertexBuffer : register(t2);
RWStructuredBuffer<uint3> outputSolverAccumulationBuffer : register(u0);
RWStructuredBuffer<uint> outputSolverCountBuffer : register(u1);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint constraintID = dispatchThreadID.x;
    
    uint id0 = inputEdgeBuffer[constraintID].vertexA;
    uint id1 = inputEdgeBuffer[constraintID].vertexB;
    
    float3 position0 = inputVertexBuffer[id0].position;
    float3 position1 = inputVertexBuffer[id1].position;

    float3 displacement = position0 - position1;
    float magnitude = length(displacement);
    
    float restLength = inputEdgeConstraintBuffer[constraintID];
    float3 solve = displacement * (1.0 - restLength / magnitude);

    uint quantizedX = (uint) (solve.x * QUANTIZE);
    uint quantizedY = (uint) (solve.y * QUANTIZE);
    uint quantizedZ = (uint) (solve.z * QUANTIZE);
    uint invQuantizedX = (uint) (-solve.x * QUANTIZE);
    uint invQuantizedY = (uint) (-solve.y * QUANTIZE);
    uint invQuantizedZ = (uint) (-solve.z * QUANTIZE);
    
    InterlockedAdd(outputSolverAccumulationBuffer[id0].x, invQuantizedX * 0.5);
    InterlockedAdd(outputSolverAccumulationBuffer[id0].y, invQuantizedY * 0.5);
    InterlockedAdd(outputSolverAccumulationBuffer[id0].z, invQuantizedZ * 0.5);
    InterlockedAdd(outputSolverAccumulationBuffer[id1].x, quantizedX * 0.5);
    InterlockedAdd(outputSolverAccumulationBuffer[id1].y, quantizedY * 0.5);
    InterlockedAdd(outputSolverAccumulationBuffer[id1].z, quantizedZ * 0.5);

    InterlockedAdd(outputSolverCountBuffer[id0], 1);
    InterlockedAdd(outputSolverCountBuffer[id1], 1);
}