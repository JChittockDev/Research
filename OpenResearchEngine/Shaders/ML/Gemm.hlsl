// GemmCS.hlsl
cbuffer GemmCB : register(b0)
{
    uint M; // rows of A and C
    uint N; // cols of B and C
    uint K; // cols of A / rows of B
    uint hasBias; // 0/1
};

StructuredBuffer<float> A : register(t0); // [M*K], row-major
StructuredBuffer<float> B : register(t1); // [K*N], row-major
StructuredBuffer<float> Bias : register(t2); // [N], optional (hasBias==1)
RWStructuredBuffer<float> C : register(u0); // [M*N], row-major

// We use tid.x = m (row), tid.y = n (col)
[numthreads(16, 16, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    const uint m = tid.x;
    const uint n = tid.y;

    if (m >= M || n >= N)
        return;

    float sum = 0.0f;
    if (hasBias != 0)
        sum = Bias[n];

    [loop]
    for (uint k = 0; k < K; ++k)
    {
        sum += A[m * K + k] * B[k * N + n];
    }

    C[m * N + n] = sum;
}
