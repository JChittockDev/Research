// BiasAddNCHWCS.hlsl
cbuffer BiasCB : register(b0)
{
    uint N, C, H, W;
};

StructuredBuffer<float> X    : register(t0); // [N*C*H*W]
StructuredBuffer<float> Bias : register(t1); // [C]
RWStructuredBuffer<float> Y  : register(u0); // [N*C*H*W]

[numthreads(64,1,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint total = N*C*H*W;
    if (idx >= total) return;

    // Recover c index from flat id
    uint hw = H*W;
    uint n  = idx / (C*hw);
    uint rem= idx % (C*hw);
    uint c  = rem / hw;

    Y[idx] = X[idx] + Bias[c];
}
