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

struct RestConstraint
{
    float length[8];
};

StructuredBuffer<RestConstraint> restConstraintBuffer : register(t0);
StructuredBuffer<Neighbours> simMeshVertexAdjacencyBuffer : register(t1);
RWStructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(u0);

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int springIterations = 1;
    int neighbourCount = 4;
    uint vertexID = dispatchThreadID.x;
    
    // Solve for the current frame
    for (int iter = 0; iter < springIterations; ++iter)
    {
        for (int ni = 0; ni < neighbourCount; ++ni)
        {
            uint neighbour = simMeshVertexAdjacencyBuffer[vertexID].index[ni];
            float neighbourLength = restConstraintBuffer[vertexID].length[ni];
        
            if (neighbour != vertexID)
            {
                float3 neighbourSkinnedDisplacement = simMeshTransformedVertexBuffer[vertexID].position - simMeshTransformedVertexBuffer[neighbour].position;
                float neighbourSkinnedLength = length(neighbourSkinnedDisplacement);
                
                // Calculate scale and correction vector
                float3 correctionVector = (neighbourSkinnedDisplacement * (1.0 - neighbourLength / neighbourSkinnedLength)) * 2.0;
                
                simMeshTransformedVertexBuffer[vertexID].position += -correctionVector;
                //simMeshTransformedVertexBuffer[neighbour].position += correctionVector;
            }
        }
    }
}
