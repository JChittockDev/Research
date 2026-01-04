// AddCS.hlsl
cbuffer AddCB : register(b0)
{
    uint Count; // number of elements
};

StructuredBuffer<float> A : register(t0);
StructuredBuffer<float> B : register(t1);
RWStructuredBuffer<float> Out : register(u0);

[numthreads(64,1,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if (i >= Count) return;
    Out[i] = A[i] + B[i];
}
