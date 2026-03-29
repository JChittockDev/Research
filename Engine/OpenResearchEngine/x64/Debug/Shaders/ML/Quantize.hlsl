// QuantizeInt8CS.hlsl  (float32 -> int8 with scale/zero_point)
cbuffer QCB : register(b0)
{
    uint Count;
    float scale;       // > 0
    int   zero_point;  // typically in [-128,127]
    float _pad0;
};

StructuredBuffer<float> X : register(t0);
RWStructuredBuffer<int>  Y : register(u0); // store as 32-bit int (only low 8 bits used)

[numthreads(64,1,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    if (i >= Count) return;
    float q = round(X[i] / scale) + float(zero_point);
    q = clamp(q, -128.0, 127.0);
    int qi = (int)q & 0xFF; // keep range
    Y[i] = qi;
}
