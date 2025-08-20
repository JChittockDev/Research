cbuffer cbSsao : register(b0)
{
    float4x4 gProj; //64
    float4 gBlurWeights[3]; //48
    float2 gInvRenderTargetSize; //8
    int gBlurRadius; //4
    uint gHorizontalBlur; //4
    uint gPadding1; //4
    uint gPadding2; // 4
};
Texture2D gDepthMap : register(t0);
Texture2D gInputMap : register(t1);
 
SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepth : register(s7);
 
static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};
 
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;
    vout.TexC = gTexCoords[vid];
    vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Poisson disk samples - 16 sample pattern
    static const float2 poissonDisk[16] =
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };
    
    // Sample center
    float4 color = gInputMap.SampleLevel(gsamLinearClamp, pin.TexC, 0);
    float totalWeight = 1.0;
    
    // Blur radius in texture coordinates
    float2 blurRadius = float2(gBlurRadius, gBlurRadius) * gInvRenderTargetSize;
    
    // Sample using Poisson disk
    for (int i = 0; i < 16; i++)
    {
        float2 sampleUV = pin.TexC + poissonDisk[i] * blurRadius;
        
        // Clamp to texture bounds
        sampleUV = saturate(sampleUV);
        
        float4 sampleColor = gInputMap.SampleLevel(gsamLinearClamp, sampleUV, 0);
        
        // Simple uniform weighting (you can add distance-based weighting if needed)
        float weight = 1.0;
        
        color += sampleColor * weight;
        totalWeight += weight;
    }
    
    return color / totalWeight;
}