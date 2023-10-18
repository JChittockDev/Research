struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<float3> inputSolverAccumulationBuffer : register(t0);
StructuredBuffer<uint> inputSolverCountBuffer : register(t1);
StructuredBuffer<Vertex> inputVertexBuffer : register(t2);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
    
    float3 solverAccumulation = inputSolverAccumulationBuffer[vertexID];
    uint solverCount = inputSolverCountBuffer[vertexID];
    float3 position = inputVertexBuffer[vertexID].position;
    
    outputVertexBuffer[vertexID].position = position + solverAccumulation / (float) solverCount;
}