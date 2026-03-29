// AvgPool2DCS.hlsl (same CB + helpers as MaxPool)
cbuffer PoolCB : register(b0)
{
    uint N, C, H, W;
    uint KH, KW;
    uint OH, OW;
    uint strideH, strideW;
    uint padH, padW;
};

StructuredBuffer<float> X : register(t0); // [N*C*H*W]
RWStructuredBuffer<float> Y : register(u0); // [N*C*OH*OW]

uint idx(uint n, uint c, uint h, uint w, uint C, uint H, uint W) { return (((n * C + c) * H + h) * W + w); }

[numthreads(8,8,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint ow = tid.x, oh = tid.y, nc = tid.z;
    if (ow >= OW || oh >= OH || nc >= N*C) return;
    uint n = nc / C, c = nc % C;

    float sum = 0.0f;
    float cnt = 0.0f;

    int y0 = int(oh)*int(strideH) - int(padH);
    int x0 = int(ow)*int(strideW) - int(padW);

    [loop]
    for (uint ky=0; ky<KH; ++ky){
        int iy = y0 + int(ky);
        if (iy < 0 || iy >= int(H)) continue;
        [loop]
        for (uint kx=0; kx<KW; ++kx){
            int ix = x0 + int(kx);
            if (ix < 0 || ix >= int(W)) continue;
            sum += X[idx(n, c, iy, ix, C, H, W)];
            cnt += 1.0f;
        }
    }
    Y[idx(n, c, oh, ow, C, OH, OW)] = (cnt > 0.0f) ? (sum / cnt) : 0.0f;
}
