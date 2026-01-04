// Conv2DCS.hlsl  (NCHW)
// Weight layout: [OC, IC, KH, KW] in row-major (OC fastest at outermost loop)
// Input  layout: [N, IC, H, W]
// Output layout: [N, OC, OH, OW]
cbuffer ConvCB : register(b0)
{
    uint N;           // batch
    uint IC;          // in channels
    uint OC;          // out channels
    uint H, W;        // input H,W
    uint KH, KW;      // kernel H,W
    uint OH, OW;      // output H,W
    uint padH, padW;
    uint strideH, strideW;
    uint dilH, dilW;
    uint hasBias;     // 0/1
};

StructuredBuffer<float> X      : register(t0);   // [N*IC*H*W]
StructuredBuffer<float> Weight : register(t1);   // [OC*IC*KH*KW]
StructuredBuffer<float> Bias   : register(t2);   // [OC] (optional)
RWStructuredBuffer<float> Y    : register(u0);   // [N*OC*OH*OW]

// flatten helpers
uint idxNCHW(uint n, uint c, uint h, uint w, uint C, uint H, uint W) 
{ 
	return (((n*C + c)*H + h)*W + w); 
}

uint idxW(uint oc, uint ic, uint ky, uint kx, uint IC, uint KH, uint KW)
{ 
	return (((oc*IC + ic)*KH + ky)*KW + kx); 
}

[numthreads(8, 8, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    const uint ow = tid.x;
    const uint oh = tid.y;
    const uint noc = tid.z; // pack (n,oc) into z if you dispatch z = N*OC

    if (ow >= OW || oh >= OH || noc >= N*OC) return;

    const uint n  = noc / OC;
    const uint oc = noc % OC;

    float acc = (hasBias != 0) ? Bias[oc] : 0.0f;

    // map output -> input top-left
    const int in_y0 = int(oh) * int(strideH) - int(padH);
    const int in_x0 = int(ow) * int(strideW) - int(padW);

    // convolution sum
    [loop]
    for (uint ic = 0; ic < IC; ++ic)
    {
        [loop]
        for (uint ky = 0; ky < KH; ++ky)
        {
            const int iy = in_y0 + int(ky) * int(dilH);
            if (iy < 0 || iy >= int(H)) continue;

            [loop]
            for (uint kx = 0; kx < KW; ++kx)
            {
                const int ix = in_x0 + int(kx) * int(dilW);
                if (ix < 0 || ix >= int(W)) continue;

                float x = X[idxNCHW(n, ic, iy, ix, IC, H, W)];
                float w = Weight[idxW(oc, ic, ky, kx, IC, KH, KW)];
                acc += x * w;
            }
        }
    }

    Y[idxNCHW(n, oc, oh, ow, OC, OH, OW)] = acc;
}
