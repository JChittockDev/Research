struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct Neighbours
{
    uint index[8];
};

struct Spring
{
    float3 transform;
};

StructuredBuffer<Vertex> inputVertexBuffer : register(t0);
StructuredBuffer<Vertex> skinnedVertexBuffer : register(t1);
StructuredBuffer<Neighbours> vertexAdjacencyBuffer : register(t2);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);
RWStructuredBuffer<Spring> springTransformBuffer : register(u1);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int springIterations = 4;
    int neighbourCount = 8;
    
    uint vertexID = dispatchThreadID.x;
   
    float3 inputVertexPosition = inputVertexBuffer[vertexID].position;
    float3 skinnedVertexPosition = skinnedVertexBuffer[vertexID].position;
    outputVertexBuffer[vertexID].position = skinnedVertexPosition + springTransformBuffer[vertexID].transform;
    
    // Set the Pre Solve init positions for the outputBuffer
    for (int i = 0; i < 8; ++i)
    {
        uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
        if (neighbour != vertexID)
        {
            outputVertexBuffer[neighbour].position = skinnedVertexBuffer[neighbour].position + springTransformBuffer[neighbour].transform;
        }
    }
    
    // Apply the constraints
    for (int i = 0; i < springIterations; ++i)
    {
        for (int i = 0; i < neighbourCount; ++i)
        {
            uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
            if (neighbour != vertexID)
            {
            
                float3 constraintDisplacement = inputVertexBuffer[vertexID].position - inputVertexBuffer[neighbour].position;
                float3 constraintSkinnedDisplacement = outputVertexBuffer[vertexID].position - outputVertexBuffer[neighbour].position;
            
                float constraintLength = length(constraintDisplacement);
                float constraintSkinnedLength = length(constraintSkinnedDisplacement);
                
                float scale = constraintLength / constraintSkinnedLength;
            
                float3 correctionVector = (constraintSkinnedDisplacement * (1.0 - scale)) * 0.5;
            
                if (scale > 1e-6)
                {
                    outputVertexBuffer[vertexID].position += -correctionVector;
                    outputVertexBuffer[neighbour].position += correctionVector;
                }
            }
        }
    }
    
    // Pass the relative solver data to the spring transform buffer so it can initialize the next frame
    for (int i = 0; i < 8; ++i)
    {
        uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
        if (neighbour != vertexID)
        {
            float3 constraintSkinnedVertexPosition = skinnedVertexBuffer[neighbour].position;
            
            springTransformBuffer[vertexID].transform = outputVertexBuffer[vertexID].position - skinnedVertexPosition;
            springTransformBuffer[neighbour].transform = outputVertexBuffer[neighbour].position - constraintSkinnedVertexPosition;

        }
    }
}