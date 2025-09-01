// GemmCS.hlsl
cbuffer GemmCB : register(b0)
{
    uint M, N, K;
};

StructuredBuffer<float> A : register(t0); // [M,K] input
StructuredBuffer<float> B : register(t1); // [K,N] weights
StructuredBuffer<float> Bias : register(t2); // [N]
RWStructuredBuffer<float> C : register(u0); // [M,N]

[numthreads(16, 16, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < M && tid.y < N)
    {
        float sum = Bias[tid.y];
        for (uint k = 0; k < K; ++k)
        {
            sum += A[tid.x * K + k] * B[k * N + tid.y];
        }
        C[tid.x * N + tid.y] = sum;
    }
}
