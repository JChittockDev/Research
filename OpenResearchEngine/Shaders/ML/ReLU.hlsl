// ReluCS.hlsl
StructuredBuffer<float> In : register(t0);
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    Out[i] = max(0.0f, In[i]);
}
