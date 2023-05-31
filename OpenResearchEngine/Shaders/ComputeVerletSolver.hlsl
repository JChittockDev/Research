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
   
    // Retrieve the input and skinned vertex positions
    float3 inputVertexPosition = inputVertexBuffer[vertexID].position;
    float3 skinnedVertexPosition = skinnedVertexBuffer[vertexID].position;
    
    // add the previous frames solve to the new skinned position to initialize the current frames solve
    outputVertexBuffer[vertexID].position = skinnedVertexPosition + springTransformBuffer[vertexID].transform;
    
    // Set the Pre Solve init positions for the outputBuffer
    for (int i = 0; i < neighbourCount; ++i)
    {
        uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
        if (neighbour != vertexID)
        {
            // Set the initial positions for neighboring vertices to prepare for the current frames solve
            outputVertexBuffer[neighbour].position = skinnedVertexBuffer[neighbour].position + springTransformBuffer[neighbour].transform;
        }
    }
    
    // Solve for the current frame
    for (int i = 0; i < springIterations; ++i)
    {
        for (int i = 0; i < neighbourCount; ++i)
        {
            uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
            if (neighbour != vertexID)
            {
                // Calculate displacement and lengths for constraints
                float3 constraintDisplacement = inputVertexBuffer[vertexID].position - inputVertexBuffer[neighbour].position;
                float3 constraintSkinnedDisplacement = outputVertexBuffer[vertexID].position - outputVertexBuffer[neighbour].position;
            
                float constraintLength = length(constraintDisplacement);
                float constraintSkinnedLength = length(constraintSkinnedDisplacement);
                
                // Calculate scale and correction vector
                float scale = constraintLength / constraintSkinnedLength;
                float3 correctionVector = (constraintSkinnedDisplacement * (1.0 - scale)) * 0.5;
            
                // Apply the correction vectors if scale is greater than epsilon
                if (scale > 1e-6)
                {
                    // Apply the correction vectors to the positions of the vertices
                    outputVertexBuffer[vertexID].position += -correctionVector;
                    outputVertexBuffer[neighbour].position += correctionVector;
                }
            }
        }
    }
    
    // Pass the relative solver data to the spring transform buffer so it can initialize the solve during the next frame
    for (int i = 0; i < neighbourCount; ++i)
    {
        uint neighbour = vertexAdjacencyBuffer[vertexID].index[i];
        
        if (neighbour != vertexID)
        {
            // Calculate the constraint skinned vertex position
            float3 constraintSkinnedVertexPosition = skinnedVertexBuffer[neighbour].position;
            
            // Update the spring transform buffer with the relative transformations
            springTransformBuffer[vertexID].transform = outputVertexBuffer[vertexID].position - skinnedVertexPosition;
            springTransformBuffer[neighbour].transform = outputVertexBuffer[neighbour].position - constraintSkinnedVertexPosition;
        }
    }
}
