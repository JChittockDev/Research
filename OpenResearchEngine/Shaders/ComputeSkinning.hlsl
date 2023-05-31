// Define your shader input and output structures
struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct SkinningInfo
{
    float4 boneWeights;
    float4 boneIndices;
};

// Define the constant buffer structure
cbuffer cbCompute : register(b0)
{
    float4x4 boneMatrices[55]; // Maximum number of bone matrices
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<SkinningInfo> skinningBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int vertexID = dispatchThreadID.x;
    Vertex inputVertex = inputVertexBuffer[vertexID];
    SkinningInfo skinning = skinningBuffer[vertexID];

    float3 position = float3(0.0f, 0.0f, 0.0f);
    float3 normal = float3(0.0f, 0.0f, 0.0f);
    float3 tangent = float4(0.0f, 0.0f, 0.0f, 0.f);
    
    for(int i = 0; i < 4; ++i)
    {
        position += skinning.boneWeights[i] * mul(float4(inputVertex.position, 1.0f), boneMatrices[(int)skinning.boneIndices[i]]).xyz;
        normal += skinning.boneWeights[i] * mul(inputVertex.normal, (float3x3) boneMatrices[(int)skinning.boneIndices[i]]);
        tangent += skinning.boneWeights[i] * mul(inputVertex.tangent.xyz, (float3x3) boneMatrices[(int)skinning.boneIndices[i]]);
    }

    outputVertexBuffer[vertexID].position = position;
    outputVertexBuffer[vertexID].normal = normal;
    outputVertexBuffer[vertexID].tangent = float4(tangent, 0.0f);
}