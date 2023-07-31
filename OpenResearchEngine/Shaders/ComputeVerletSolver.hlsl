struct Vertex
{
    float3 position;
    float3 normal;
    float2 texCoord;
    float4 tangent;
};

struct Edge
{
    uint vertexA;
    uint vertexB;
};

StructuredBuffer<Vertex> simMeshTransformedVertexBuffer : register(t0);
StructuredBuffer<Edge> simMeshEdgeBuffer : register(t1);
StructuredBuffer<float> simMeshRestLengthBuffer : register(t2);
RWStructuredBuffer<uint3> simMeshSolverTransformBuffer : register(u0);
RWStructuredBuffer<uint> simMeshSolverCountBuffer : register(u1);

void DistanceConstraint(float3 point1, float3 point2, float restLength, out float3 correction1, out float3 correction2)
{
    correction1 = float3(0, 0, 0);
    correction2 = float3(0, 0, 0);
    float3 displacement = point1 - point2;
    float len = length(displacement);
    float3 correction = displacement * (1.0 - restLength / len);
    correction1 = -correction * 0.5;
    correction2 = correction * 0.5;
}

#define QUANTIZE 32768.0

// Define the compute shader entry point
[numthreads(1, 1, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int springIterations = 1;
    int neighbourCount = 4;
    uint edgeID = dispatchThreadID.x;
    
    Edge currentEdge = simMeshEdgeBuffer[edgeID];
    
    Vertex vertexA = simMeshTransformedVertexBuffer[currentEdge.vertexA];
    Vertex vertexB = simMeshTransformedVertexBuffer[currentEdge.vertexB];
    
    float3 correction1 = float3(0.0, 0.0, 0.0);
    float3 correction2 = float3(0.0, 0.0, 0.0);
    
    DistanceConstraint(vertexA.position, vertexB.position, simMeshRestLengthBuffer[edgeID], correction1, correction2);
    
    uint invQuantizedX = (uint) (correction1.x * QUANTIZE);
    uint invQuantizedY = (uint) (correction1.y * QUANTIZE);
    uint invQuantizedZ = (uint) (correction1.z * QUANTIZE);
    uint quantizedX = (uint) (correction2.x * QUANTIZE);
    uint quantizedY = (uint) (correction2.y * QUANTIZE);
    uint quantizedZ = (uint) (correction2.z * QUANTIZE);

    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexA].x, invQuantizedX);
    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexA].y, invQuantizedY);
    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexA].z, invQuantizedZ);
    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexB].x, quantizedX);
    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexB].y, quantizedY);
    InterlockedAdd(simMeshSolverTransformBuffer[currentEdge.vertexB].z, quantizedZ);
    
    InterlockedAdd(simMeshSolverCountBuffer[currentEdge.vertexA], 1);
    InterlockedAdd(simMeshSolverCountBuffer[currentEdge.vertexB], 1);
}