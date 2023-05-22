struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 tangent : TANGENT;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<Vertex> skinnedVertexBuffer : register(t1);
StructuredBuffer<int> adjacencyBuffer : register(t2);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int vertexID = dispatchThreadID.x;
    
    int constraintVertices[8];
    constraintVertices[0] = adjacencyBuffer[vertexID * 8 + 0];
    constraintVertices[1] = adjacencyBuffer[vertexID * 8 + 1];
    constraintVertices[2] = adjacencyBuffer[vertexID * 8 + 2];
    constraintVertices[3] = adjacencyBuffer[vertexID * 8 + 3];
    constraintVertices[4] = adjacencyBuffer[vertexID * 8 + 4];
    constraintVertices[5] = adjacencyBuffer[vertexID * 8 + 5];
    constraintVertices[6] = adjacencyBuffer[vertexID * 8 + 6];
    constraintVertices[7] = adjacencyBuffer[vertexID * 8 + 7];
    
    Vertex inputVertex = inputVertexBuffer[vertexID];
    Vertex skinnedVertex = skinnedVertexBuffer[vertexID];
    
    float3 displacement = inputVertex.position - skinnedVertex.position;
    float displacementLength = length(displacement);
    
    for (int i = 0; i < 8; ++i)
    {
        Vertex constraintInputVertex = inputVertexBuffer[constraintVertices[i]];
        Vertex constraintSkinnedVertex = skinnedVertexBuffer[constraintVertices[i]];
        
        //float3 constraintDisplacement = inputVertex.position - constraintInputVertex.position;
        //float3 constraintSkinnedDisplacement = skinnedVertex.position - constraintSkinnedVertex.position;
        
        //float constraintLength = length(constraintDisplacement);
        //float constraintSkinnedLength = length(constraintSkinnedDisplacement);
        
        //float3 correctionVector = (constraintSkinnedDisplacement * (1 - displacementLength / constraintSkinnedLength)) * 0.5;
        
        //outputVertexBuffer[vertexID].position = skinnedVertex.position + correctionVector;
        //outputVertexBuffer[constraintVertices[i]].position = constraintSkinnedVertex.position - correctionVector;
        
        outputVertexBuffer[vertexID].position = skinnedVertex.position;
        outputVertexBuffer[constraintVertices[i]].position = constraintSkinnedVertex.position;
    }
    
    outputVertexBuffer[vertexID].position = skinnedVertex.position;
    outputVertexBuffer[vertexID].normal = skinnedVertex.normal;
    outputVertexBuffer[vertexID].tangent = skinnedVertex.tangent;
}