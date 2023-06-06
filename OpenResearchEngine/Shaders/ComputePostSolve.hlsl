struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct InterlockedVector
{
    int x;
    int y;
    int z;
};

StructuredBuffer<InterlockedVector> simMeshTransformedVertexBuffer : register(t0);
RWStructuredBuffer<Vertex> simMeshPostSolveVertexBuffer : register(u0);

float3 UnQuantize(InterlockedVector input, float factor)
{
    float vertexPositionX = ((float) input.x) / factor;
    float vertexPositionY = ((float) input.y) / factor;
    float vertexPositionZ = ((float) input.z) / factor;
    return float3(vertexPositionX, vertexPositionY, vertexPositionZ);
}

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint simMeshVertexID = dispatchThreadID.x;
    
    float3 vertexPosition = UnQuantize(simMeshTransformedVertexBuffer[simMeshVertexID], QUANTIZE);
    simMeshPostSolveVertexBuffer[simMeshVertexID].position = vertexPosition;
}
