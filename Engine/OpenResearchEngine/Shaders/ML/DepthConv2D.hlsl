// DepthwiseConv2DCS.hlsl (NCHW, depth_multiplier = 1)
cbuffer DWConvCB : register(b0)
{
    uint N, C, H, W;
    uint KH, KW;
    uint OH, OW;
    uint padH, padW;
    uint strideH, strideW;
    uint dilH, dilW;
    uint hasBias;
};

StructuredBuffer<float> X      : register(t0); // [N*C*H*W]
StructuredBuffer<float> Weight : register(t1); // [C*KH*KW]
StructuredBuffer<float> Bias   : register(t2); // [C]
RWStructuredBuffer<float> Y    : register(u0); // [N*C*OH*OW]

uint idxNCHW(uint n, uint c, uint h, uint w, uint C, uint H, uint W)
{ 
    return (((n*C + c)*H + h)*W + w); 
}

uint idxW(uint c, uint ky, uint kx, uint KH, uint KW)
{ 
    return ((c*KH + ky)*KW + kx); 
}

[numthreads(8, 8, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    const uint ow = tid.x;
    const uint oh = tid.y;
    const uint nc = tid.z; // n*C packed
    if (ow >= OW || oh >= OH || nc >= N*C) return;

    const uint n = nc / C;
    const uint c = nc % C;

    float acc = (hasBias != 0) ? Bias[c] : 0.0f;

    const int in_y0 = int(oh)*int(strideH) - int(padH);
    const int in_x0 = int(ow)*int(strideW) - int(padW);

    [loop]
    for (uint ky = 0; ky < KH; ++ky)
    {
        const int iy = in_y0 + int(ky)*int(dilH);
        if (iy < 0 || iy >= int(H)) continue;
        [loop]
        for (uint kx = 0; kx < KW; ++kx)
        {
            const int ix = in_x0 + int(kx)*int(dilW);
            if (ix < 0 || ix >= int(W)) continue;

            float x = X[idxNCHW(n, c, iy, ix, C, H, W)];
            float w = Weight[idxW(c, ky, kx, KH, KW)];
            acc += x * w;
        }
    }

    Y[idxNCHW(n, c, oh, ow, C, OH, OW)] = acc;
}
