struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<uint> inputEdgeIDBuffer : register(t0);
StructuredBuffer<float> inputEdgeConstraintBuffer : register(t1);
StructuredBuffer<float> invMassBuffer : register(t2);
StructuredBuffer<Vertex> inputVertexBuffer : register(t3);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float deltaT = 0.01;
    float constraintCompliance = 1.0;
    float alpha = constraintCompliance / deltaT / deltaT;
    
    uint constraintID = dispatchThreadID.x;
    
    uint id0 = inputEdgeIDBuffer[2 * constraintID];
    uint id1 = inputEdgeIDBuffer[2 * constraintID + 1];
    
    float3 position0 = inputVertexBuffer[id0].position;
    float3 position1 = inputVertexBuffer[id1].position;

    float3 displacement = position0 - position1;
    float magnitude = length(displacement);
    
    if (magnitude != 0.0)
    {
        displacement *= 1.0 / magnitude;
        float restLength = inputEdgeConstraintBuffer[constraintID];
        float lengthDiff = magnitude - restLength;
        //float scale = -lengthDiff / (w + alpha);
            
        outputVertexBuffer[id0].position = position0 + (displacement * 0.5);
        outputVertexBuffer[id1].position = position1 + (-displacement * 0.5);
    }
}