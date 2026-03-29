// ConcatCHWNCS.hlsl  (concatenate A and B along C dimension)
cbuffer ConcatCB : register(b0)
{
    uint N, C1, C2, H, W; // result C = C1 + C2
};

StructuredBuffer<float> A : register(t0); // [N*C1*H*W]
StructuredBuffer<float> B : register(t1); // [N*C2*H*W]
RWStructuredBuffer<float> Y : register(u0); // [N*(C1+C2)*H*W]

uint idxNCHW(uint n, uint c, uint h, uint w, uint C, uint H, uint W)
{ 
    return (((n*C + c)*H + h)*W + w); 
}

[numthreads(8,8,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint w = tid.x, h = tid.y, nc = tid.z; // pack n*c in z for throughput
    uint C = C1 + C2;
    if (w >= W || h >= H || nc >= N*C) return;

    uint n = nc / C;
    uint c = nc % C;

    if (c < C1)
        Y[idxNCHW(n, c, h, w, C, H, W)] = A[idxNCHW(n, c, h, w, C1, H, W)];
    else {
        uint c2 = c - C1;
        Y[idxNCHW(n, c, h, w, C, H, W)] = B[idxNCHW(n, c2, h, w, C2, H, W)];
    }
}
