// SoftmaxCS.hlsl
cbuffer SoftmaxCB : register(b0)
{
    uint M, N;
};

StructuredBuffer<float> In : register(t0); // [M,N]
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < M)
    {
        // Find max
        float maxVal = -3.4e38;
        for (uint j = 0; j < N; ++j)
            maxVal = max(maxVal, In[tid.x * N + j]);

        // Compute exp & sum
        float sum = 0.0f;
        for (uint j = 0; j < N; ++j)
            sum += exp(In[tid.x * N + j] - maxVal);

        // Normalize
        for (uint j = 0; j < N; ++j)
            Out[tid.x * N + j] = exp(In[tid.x * N + j] - maxVal) / sum;
    }
}
