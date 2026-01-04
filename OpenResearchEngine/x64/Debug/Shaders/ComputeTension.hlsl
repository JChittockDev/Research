struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct VertexNeighbours
{
    uint index[8];
    float length[8];
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<VertexNeighbours> vertexNeighbourBuffer : register(t1);
RWStructuredBuffer<float4> tensionBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;

    VertexNeighbours vertexNeighbours = vertexNeighbourBuffer[vertexID];
    
    float3 baseVertexPosition = inputVertexBuffer[vertexID].position;
    
    float4 sumCoefficient = float4(0.0, 0.0, 0.0, 0.0);
    
    uint divisor = 0;
    
    for (uint i = 0; i < 8; ++i)
    {
        if (vertexNeighbours.index[i] != vertexID)
        {
            float3 secondaryVertexPosition = inputVertexBuffer[vertexNeighbours.index[i]].position;
            float3 displacement = secondaryVertexPosition - baseVertexPosition;
            float magnitude = length(displacement);
            float restLength = vertexNeighbours.length[i];
            float displacementCoefficient = 1.0 - restLength / magnitude;
            sumCoefficient += float4(displacement.x, displacement.y, displacement.z, displacementCoefficient);
            divisor += 1;
        }
    }
    
    tensionBuffer[vertexID] = sumCoefficient / divisor;
}