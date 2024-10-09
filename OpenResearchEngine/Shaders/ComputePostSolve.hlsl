struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<float4> vertexColor : register(t0);
StructuredBuffer<float4> tensionBuffer : register(t1);
StructuredBuffer<uint3> inputSolverAccumulationBuffer : register(t2);
StructuredBuffer<uint> inputSolverCountBuffer : register(t3);
StructuredBuffer<Vertex> inputVertexBuffer : register(t4);
StructuredBuffer<Vertex> initVertexBuffer : register(t5);
StructuredBuffer<Vertex> skinnedVertexBuffer : register(t6);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
   
    float coefficient = abs(tensionBuffer[vertexID].w);
    
    float4 vertexColorData = vertexColor[vertexID];
    
    float coeffOffset = 0.0;
    
    if (coefficient > 1.0)
    {
        coeffOffset = coefficient - 1.0;

    }
    else
    {
        coeffOffset = 1.0 - coefficient;
    }
    
    coeffOffset = clamp(coeffOffset * 1.4, 0.0, 1.0) * 0.95 * vertexColorData.x;
    
    uint3 solverAccumulation = inputSolverAccumulationBuffer[vertexID];

    float vertexPositionX = ((float) solverAccumulation.x) / QUANTIZE;
    float vertexPositionY = ((float) solverAccumulation.y) / QUANTIZE;
    float vertexPositionZ = ((float) solverAccumulation.z) / QUANTIZE;
    float3 floatSolverAccumulation = float3(vertexPositionX, vertexPositionY, vertexPositionZ);
    
    
    uint solverCount = inputSolverCountBuffer[vertexID];
    float3 position = inputVertexBuffer[vertexID].position;
    
    float3 initPosition = initVertexBuffer[vertexID].position;
    float3 skinnedPosition = skinnedVertexBuffer[vertexID].position;
        
    outputVertexBuffer[vertexID].position = skinnedPosition * (1.0 - coeffOffset) + (position + floatSolverAccumulation / (float) solverCount) * (coeffOffset);
}