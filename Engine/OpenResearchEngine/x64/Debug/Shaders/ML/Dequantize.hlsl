// DequantizeInt8CS.hlsl  (int8 -> float32)
cbuffer DQCB : register(b0)
{
    uint Count;
    float scale;
    int   zero_point;
    float _pad0;
};

StructuredBuffer<int>  X : register(t0); // stored as int32, low 8 bits are the int8 value
RWStructuredBuffer<float> Y : register(u0);

[numthreads(64,1,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if (i >= Count) return;
    // sign-extend low 8 bits
    int8_t qi = (int8_t)(X[i] & 0xFF);
    Y[i] = (float(qi) - float(zero_point)) * scale;
}
