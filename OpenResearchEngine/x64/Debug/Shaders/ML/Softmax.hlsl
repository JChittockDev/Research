// SoftmaxCS.hlsl
cbuffer SoftmaxCB : register(b0)
{
    uint M; // rows
    uint N; // cols
    uint _pad0, _pad1;
};

StructuredBuffer<float> In : register(t0); // [M*N]
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint row = tid.x; // one thread per row
    if (row >= M)
        return;

    // 1) max
    float maxVal = -3.402823e38f;
    for (uint j = 0; j < N; ++j)
        maxVal = max(maxVal, In[row * N + j]);

    // 2) sum exp
    float sum = 0.0f;
    for (uint j = 0; j < N; ++j)
        sum += exp(In[row * N + j] - maxVal);

    // 3) normalize
    for (uint j = 0; j < N; ++j)
        Out[row * N + j] = exp(In[row * N + j] - maxVal) / sum;
}
