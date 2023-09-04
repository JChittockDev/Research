struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<float3> inputVelocityBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);
RWStructuredBuffer<float3> outputVelocityBuffer : register(u1);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float3 gravity = float3(0.0, -10.0, 0.0);
    float deltaT = 0.00001;
    
    int vertexID = dispatchThreadID.x;
    
    float3 position = inputVertexBuffer[vertexID].position;
    float3 velocity = inputVelocityBuffer[vertexID];
    
    if (vertexID < 110)
    {
        outputVertexBuffer[vertexID].position = position + gravity * deltaT;
    }
}