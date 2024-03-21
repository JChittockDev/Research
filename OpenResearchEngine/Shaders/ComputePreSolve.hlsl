struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

StructuredBuffer<float3> forceBuffer : register(t0);
StructuredBuffer<Vertex> inputVertexBuffer : register(t1);
RWStructuredBuffer<Vertex> outputVertexBuffer : register(u0);

float3 resolveSphereCollision(float3 originalPoint,
                              float3 sphereCenter,
                              float sphereRadius)
{
    // Calculate the vector from the sphere center to the original point
    float vectorX = originalPoint.x - sphereCenter.x;
    float vectorY = originalPoint.y - sphereCenter.y;
    float vectorZ = originalPoint.z - sphereCenter.z;

    // Calculate the distance between the original point and the sphere center
    float distance = sqrt(vectorX * vectorX + vectorY * vectorY + vectorZ * vectorZ);
    
    float3 outPoint = originalPoint;
    
    if (distance <= sphereRadius)
    {
        float scaleFactor = sphereRadius / distance;
        float newX = sphereCenter.x + vectorX * scaleFactor;
        float newY = sphereCenter.y + vectorY * scaleFactor;
        float newZ = sphereCenter.z + vectorZ * scaleFactor;
        
        outPoint = float3(newX, newY, newZ);

    }
    
    return outPoint;
}

// Define the compute shader entry point
[numthreads(64, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int vertexID = dispatchThreadID.x;
    float3 position = inputVertexBuffer[vertexID].position;
    
    float3 preSolvedPosition = position + forceBuffer[vertexID] - float3(0.0, 0.0025, 0.0);
    
    
    
    outputVertexBuffer[vertexID].position = resolveSphereCollision(preSolvedPosition, float3(0.0, 0.0, 0.0), 5.0);
}