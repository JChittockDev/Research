// TanhCS.hlsl
cbuffer UnaryCB : register(b0)
{
    uint Count;
    float _pad0, _pad1, _pad2;
};

StructuredBuffer<float> In : register(t0);
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if (i >= Count)
        return;

    float x = In[i];
    // numerically stable tanh is fine with exp here for typical ranges
    float e2x = exp(2.0f * x);
    Out[i] = (e2x - 1.0f) / (e2x + 1.0f);
}
