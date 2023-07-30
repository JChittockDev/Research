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

StructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(t0);
StructuredBuffer<Edge> simMeshEdgeBuffer : register(t1);
StructuredBuffer<float> simMeshRestLengthBuffer : register(t2);
RWStructuredBuffer<uint3> simMeshSpringTransformBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int springIterations = 1;
    int neighbourCount = 4;
    uint vertexID = dispatchThreadID.x;
}
