// LeakyReluCS.hlsl
cbuffer ActCB : register(b0)
{
    float alpha; // e.g., 0.01
};

StructuredBuffer<float> In : register(t0);
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    float x = In[i];
    Out[i] = (x > 0.0f) ? x : alpha * x;
}
