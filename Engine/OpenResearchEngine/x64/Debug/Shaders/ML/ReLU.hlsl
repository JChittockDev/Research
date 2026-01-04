// ReluCS.hlsl
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
    Out[i] = max(0.0f, In[i]);
}
