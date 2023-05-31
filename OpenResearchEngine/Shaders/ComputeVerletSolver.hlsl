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
    
    float3 neighbourPositions[8];
    
    neighbourPositions[0] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[0]].position;
    neighbourPositions[1] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[1]].position;
    neighbourPositions[2] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[2]].position;
    neighbourPositions[3] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[3]].position;
    neighbourPositions[4] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[4]].position;
    neighbourPositions[5] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[5]].position;
    neighbourPositions[6] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[6]].position;
    neighbourPositions[7] = skinnedVertexBuffer[vertexAdjacencyBuffer[vertexID].index[7]].position;
    
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
        
            skinnedVertex.position -= correctionVector * 0.5;
            neighbourPositions[i] += correctionVector * 0.5;
            
        }
    }
    
    outputVertexBuffer[vertexID].position = skinnedVertex.position;
    
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[0]].position = neighbourPositions[0];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[1]].position = neighbourPositions[1];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[2]].position = neighbourPositions[2];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[3]].position = neighbourPositions[3];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[4]].position = neighbourPositions[4];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[5]].position = neighbourPositions[5];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[6]].position = neighbourPositions[6];
    outputVertexBuffer[vertexAdjacencyBuffer[vertexID].index[7]].position = neighbourPositions[7];
}