// Define your shader input and output structures
struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float3 tangent;
};

struct SkinningInfo
{
    float4 boneWeights;
    uint4 boneIndices;
};

// Define the constant buffer structure
cbuffer cbCompute : register(b0)
{
    float4x4 boneMatrices[55]; // Maximum number of bone matrices
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<SkinningInfo> skinningBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define your skinning function
Vertex SkinVertex(Vertex input, SkinningInfo skinning)
{
    Vertex output = (Vertex) 0;

    // Perform skinning calculations using bone matrices
    float4x4 skinningMatrix =
        boneMatrices[skinning.boneIndices.x] * skinning.boneWeights.x +
        boneMatrices[skinning.boneIndices.y] * skinning.boneWeights.y +
        boneMatrices[skinning.boneIndices.z] * skinning.boneWeights.z +
        boneMatrices[skinning.boneIndices.w] * skinning.boneWeights.w;

    // apply skinning transformation to position
    output.position = mul(float4(input.position, 1.0f), skinningMatrix).xyz;
    output.normal = mul(float4(input.normal, 0.0f), skinningMatrix).xyz;
    output.tangent = mul(float4(input.tangent, 0.0f), skinningMatrix).xyz;

    return output;
}

// Define the compute shader entry point
[numthreads(256, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Vertex inputVertex = inputVertexBuffer[dispatchThreadID.x];
    SkinningInfo skinning = skinningBuffer[dispatchThreadID.x];
    Vertex outputVertex = SkinVertex(inputVertex, skinning);
    outputVertexBuffer[dispatchThreadID.x] = outputVertex;
}