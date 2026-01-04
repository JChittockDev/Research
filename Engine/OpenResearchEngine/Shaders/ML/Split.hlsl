// SplitCHWNCS.hlsl  (split X -> A,B along C)
cbuffer SplitCB : register(b0)
{
    uint N, C1, C2, H, W; // input C = C1 + C2
};

StructuredBuffer<float> X : register(t0); // [N*(C1+C2)*H*W]
RWStructuredBuffer<float> A : register(u0); // [N*C1*H*W]
RWStructuredBuffer<float> B : register(u1); // [N*C2*H*W]

uint idxNCHW(uint n, uint c, uint h, uint w, uint C, uint H, uint W)
{ 
    return (((n*C + c)*H + h)*W + w); 
}

[numthreads(8,8,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint w = tid.x, h = tid.y, nc = tid.z;
    uint C = C1 + C2;
    if (w >= W || h >= H || nc >= N*C) return;

    uint n = nc / C;
    uint c = nc % C;

    float v = X[idxNCHW(n, c, h, w, C, H, W)];
    if (c < C1)
        A[idxNCHW(n, c, h, w, C1, H, W)] = v;
    else
        B[idxNCHW(n, c-C1, h, w, C2, H, W)] = v;
}
