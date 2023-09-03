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
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float3 gravity = float3(0.0, -10.0, 0.0);
    float particleMass = 0.1;
    float particleInvMass = 1.0 / 0.1;
    float springK = 2000.0;
    float restLengthHoriz;
    float restLengthVert;
    float restLengthDiag;
    float deltaT = 0.01;
    float dampingConst = 0.1;
    
    int vertexID = dispatchThreadID.x;
    
    float3 position = inputVertexBuffer[vertexID].position;
    float3 velocity = inputVelocityBuffer[vertexID];
    
    float3 force = gravity * particleMass;
    force += -dampingConst * velocity;
    
    float3 acceleration = force * particleInvMass;
    
    outputVertexBuffer[vertexID].position = position + velocity * deltaT + 0.5 * acceleration * deltaT * deltaT;
    outputVelocityBuffer[vertexID] = velocity + acceleration * deltaT;
}