// TanhCS.hlsl
StructuredBuffer<float> In : register(t0);
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    float x = In[i];
    float e2x = exp(2.0f * x);
    Out[i] = (e2x - 1.0f) / (e2x + 1.0f);
}
