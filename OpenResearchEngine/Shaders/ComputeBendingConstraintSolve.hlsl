struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<uint> inputEdgeIDBuffer : register(t0);
StructuredBuffer<float> inputEdgeConstraintBuffer : register(t1);
StructuredBuffer<Vertex> inputVertexBuffer : register(t2);
RWStructuredBuffer<float3> outputSolverAccumulationBuffer : register(u0);
RWStructuredBuffer<uint> outputSolverCountBuffer : register(u1);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint constraintID = dispatchThreadID.x;
    
    uint id0 = inputEdgeIDBuffer[4 * constraintID + 2];
    uint id1 = inputEdgeIDBuffer[4 * constraintID + 3];
    
    float3 position0 = inputVertexBuffer[id0].position;
    float3 position1 = inputVertexBuffer[id1].position;

    float3 displacement = position0 - position1;
    float magnitude = length(displacement);
    
    float restLength = inputEdgeConstraintBuffer[constraintID];
    float3 solve = displacement * (1.0 - restLength / magnitude);

    outputSolverAccumulationBuffer[id0] += -solve * 0.5;
    outputSolverAccumulationBuffer[id1] += solve * 0.5;
    outputSolverCountBuffer[id0] += 1;
    outputSolverCountBuffer[id1] += 1;
}