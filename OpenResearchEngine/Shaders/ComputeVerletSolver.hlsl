struct Neighbours
{
    uint index[8];
};

struct RestConstraint
{
    float length[8];
};

struct InterlockedVector
{
    int x;
    int y;
    int z;
};

StructuredBuffer<RestConstraint> restConstraintBuffer : register(t0);
StructuredBuffer<Neighbours> simMeshVertexAdjacencyBuffer : register(t1);
RWStructuredBuffer<InterlockedVector> simMeshTransformedVertexBuffer : register(u0);

float3 UnQuantize(InterlockedVector input, float factor)
{
    float vertexPositionX = ((float) input.x) / factor;
    float vertexPositionY = ((float) input.y) / factor;
    float vertexPositionZ = ((float) input.z) / factor;
    return float3(vertexPositionX, vertexPositionY, vertexPositionZ);
}

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int springIterations = 4;
    int neighbourCount = 8;
    uint vertexID = dispatchThreadID.x;
    
    // Solve for the current frame
    for (int iter = 0; iter < springIterations; ++iter)
    {
        for (int ni = 0; ni < neighbourCount; ++ni)
        {
            uint neighbour = simMeshVertexAdjacencyBuffer[vertexID].index[ni];
            float neighbourLength = restConstraintBuffer[vertexID].length[ni];
            
            float3 vertexPosition = UnQuantize(simMeshTransformedVertexBuffer[vertexID], QUANTIZE);
        
            if (neighbour != vertexID)
            {
                float3 neighbourVertexPosition = UnQuantize(simMeshTransformedVertexBuffer[neighbour], QUANTIZE);

                float3 neighbourSkinnedDisplacement = vertexPosition - neighbourVertexPosition;
                float neighbourSkinnedLength = length(neighbourSkinnedDisplacement);
                
                // Calculate scale and correction vector
                float3 correctionVector = (neighbourSkinnedDisplacement * (1.0 - neighbourLength / neighbourSkinnedLength)) * 0.5;
                
                int quantizedX = (int) (correctionVector.x * QUANTIZE);
                int quantizedY = (int) (correctionVector.y * QUANTIZE);
                int quantizedZ = (int) (correctionVector.z * QUANTIZE);
                int invQuantizedX = (int) (-correctionVector.x * QUANTIZE);
                int invQuantizedY = (int) (-correctionVector.y * QUANTIZE);
                int invQuantizedZ = (int) (-correctionVector.z * QUANTIZE);
 
                InterlockedAdd(simMeshTransformedVertexBuffer[vertexID].x, invQuantizedX);
                InterlockedAdd(simMeshTransformedVertexBuffer[vertexID].y, invQuantizedY);
                InterlockedAdd(simMeshTransformedVertexBuffer[vertexID].z, invQuantizedZ);
                InterlockedAdd(simMeshTransformedVertexBuffer[neighbour].x, quantizedX);
                InterlockedAdd(simMeshTransformedVertexBuffer[neighbour].y, quantizedY);
                InterlockedAdd(simMeshTransformedVertexBuffer[neighbour].z, quantizedZ);
            }
        }
    }
}
