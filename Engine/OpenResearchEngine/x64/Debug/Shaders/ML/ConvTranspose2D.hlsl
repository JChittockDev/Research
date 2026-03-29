// ConvTranspose2DCS.hlsl (NCHW)
// Implements "deconvolution" by mapping each output pixel back to contributing input positions.
cbuffer DeconvCB : register(b0)
{
    uint N, IC, OC;
    uint H, W;        // input H,W
    uint KH, KW;
    uint OH, OW;      // output H,W
    uint padH, padW;
    uint strideH, strideW;
    uint hasBias;
};

StructuredBuffer<float> X      : register(t0); // [N*IC*H*W]
StructuredBuffer<float> Weight : register(t1); // [IC*OC*KH*KW]  (note IC->OC)
StructuredBuffer<float> Bias   : register(t2); // [OC]
RWStructuredBuffer<float> Y    : register(u0); // [N*OC*OH*OW]

uint idxNCHW(uint n, uint c, uint h, uint w, uint C, uint H, uint W)
{ 
    return (((n*C + c)*H + h)*W + w); 
}

uint idxW(uint ic, uint oc, uint ky, uint kx, uint OC, uint KH, uint KW)
{ 
    return (((ic*OC + oc)*KH + ky)*KW + kx); 
}

[numthreads(8, 8, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    const uint ow = tid.x;
    const uint oh = tid.y;
    const uint noc = tid.z; // n*oc packed
    if (ow >= OW || oh >= OH || noc >= N*OC) return;

    const uint n  = noc / OC;
    const uint oc = noc % OC;

    float acc = (hasBias != 0) ? Bias[oc] : 0.0f;

    // For each output (oh,ow), find input positions that contribute:
    // oh = ih*strideH - padH + ky  =>  ih = floor((oh + padH - ky)/strideH)
    // valid if (oh + padH - ky) % strideH == 0
    [loop]
    for (uint ic = 0; ic < IC; ++ic)
    {
        [loop]
        for (uint ky = 0; ky < KH; ++ky)
        {
            int numerY = int(oh) + int(padH) - int(ky);
            if (numerY < 0) continue;
            if (numerY % int(strideH) != 0) continue;
            int ih = numerY / int(strideH);
            if (ih < 0 || ih >= int(H)) continue;

            [loop]
            for (uint kx = 0; kx < KW; ++kx)
            {
                int numerX = int(ow) + int(padW) - int(kx);
                if (numerX < 0) continue;
                if (numerX % int(strideW) != 0) continue;
                int ix = numerX / int(strideW);
                if (ix < 0 || ix >= int(W)) continue;

                float x = X[idxNCHW(n, ic, ih, ix, IC, H, W)];
                float w = Weight[idxW(ic, oc, ky, kx, OC, KH, KW)];
                acc += x * w;
            }
        }
    }

    Y[idxNCHW(n, oc, oh, ow, OC, OH, OW)] = acc;
}
