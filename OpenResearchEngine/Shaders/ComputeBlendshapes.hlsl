// Define your shader input and output structures
struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct BlendshapeVertex
{
    uint index;
    float3 position;
    float3 normal;
    float3 tangent;
    float padding1;
    float padding2;
};

uint weightIndex : register(b0);

// Define the constant buffer structure
cbuffer cbCompute : register(b1)
{
    float4 bWeight[64];
};

StructuredBuffer<BlendshapeVertex> blendshapeVertexBuffer : register(t0);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float blendWeight = bWeight[weightIndex].x;
    int subsetVertexID = dispatchThreadID.x;
    BlendshapeVertex blendshapeVertex = blendshapeVertexBuffer[subsetVertexID];
    outputVertexBuffer[blendshapeVertex.index].position += blendshapeVertex.position * blendWeight;
    outputVertexBuffer[blendshapeVertex.index].normal += blendshapeVertex.normal * blendWeight;
    outputVertexBuffer[blendshapeVertex.index].tangent += float4(blendshapeVertex.tangent, 1.0) * blendWeight;
}