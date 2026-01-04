// MaxPool2DCS.hlsl
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

uint idx(uint n, uint c, uint h, uint w, uint C, uint H, uint W){ return (((n*C + c)*H + h)*W + w); }

[numthreads(8,8,1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    uint ow = tid.x, oh = tid.y, nc = tid.z;
    if (ow >= OW || oh >= OH || nc >= N*C) return;
    uint n = nc / C, c = nc % C;

    float m = -3.402823e38f;
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
            m = max(m, X[idx(n, c, iy, ix, C, H, W)]);
        }
    }
    Y[idx(n, c, oh, ow, C, OH, OW)] = m;
}
