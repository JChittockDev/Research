struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 tangent : TANGENT;
};

struct Neighbours
{
    uint index[8];
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<Vertex> skinnedVertexBuffer : register(t1);
StructuredBuffer<Neighbours> vertexAdjacencyBuffer : register(t2);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint vertexID = dispatchThreadID.x;
    Vertex inputVertex = inputVertexBuffer[vertexID];
    Vertex skinnedVertex = skinnedVertexBuffer[vertexID];
    
    for (int i = 0; i < 8; ++i)
    {
        uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
        if (neighbour != vertexID)
        {
            Vertex constraintInputVertex = inputVertexBuffer[neighbour];
            Vertex constraintSkinnedVertex = skinnedVertexBuffer[neighbour];
        
            float3 constraintDisplacement = inputVertex.position - constraintInputVertex.position;
            float3 constraintSkinnedDisplacement = skinnedVertex.position - constraintSkinnedVertex.position;
        
            float constraintLength = length(constraintDisplacement);
            float constraintSkinnedLength = length(constraintSkinnedDisplacement);
        
            float3 correctionVector = constraintSkinnedDisplacement * (1 - constraintLength / constraintSkinnedLength);
        
            skinnedVertex.position += correctionVector;
        }
    }
    
    outputVertexBuffer[vertexID].position = skinnedVertex.position;
    outputVertexBuffer[vertexID].normal = skinnedVertex.normal;
    outputVertexBuffer[vertexID].tangent = skinnedVertex.tangent;
    outputVertexBuffer[vertexID].texCoord = skinnedVertex.texCoord;
}