cbuffer cbSsao : register(b0)
{
    float4x4 gProj; //16
    float4 gBlurWeights[3]; //16
    float2 gInvRenderTargetSize; //8
    uint gBlurRadius; //4 put this back to bool and see if the blur starts working again, look at what the constant buffer values for the other stuff is then
    uint gHorizontalBlur; //4
    uint gPadding1; //4
    uint gPadding2; // 4
};

Texture2D gNormalMap : register(t0);
Texture2D gDepthMap  : register(t1);
Texture2D gInputMap  : register(t2);
 
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
    float4 PosH  : SV_POSITION;
	float2 TexC  : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;

    vout.TexC = gTexCoords[vid];
    vout.PosH = float4(2.0f*vout.TexC.x - 1.0f, 1.0f - 2.0f*vout.TexC.y, 0.0f, 1.0f);

    return vout;
}

float NdcDepthToViewDepth(float z_ndc)
{
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}

float4 PS(VertexOut pin) : SV_Target
{
    float blurWeights[12] =
    {
        gBlurWeights[0].x, gBlurWeights[0].y, gBlurWeights[0].z, gBlurWeights[0].w,
        gBlurWeights[1].x, gBlurWeights[1].y, gBlurWeights[1].z, gBlurWeights[1].w,
        gBlurWeights[2].x, gBlurWeights[2].y, gBlurWeights[2].z, gBlurWeights[2].w,
    };

	float2 texOffset;
	if(gHorizontalBlur == 1)
	{
		texOffset = float2(gInvRenderTargetSize.x, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gInvRenderTargetSize.y);
	}

	float4 color = blurWeights[gBlurRadius] * gInputMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0);
	float totalWeight = blurWeights[gBlurRadius];
	 
    float3 centerNormal = gNormalMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz;
    float centerDepth = NdcDepthToViewDepth(gDepthMap.SampleLevel(gsamDepth, pin.TexC, 0.0f).r);

	for(float i = -gBlurRadius; i <=gBlurRadius; ++i)
	{
		if( i == 0 )
        {
            continue;
        }

		float2 tex = pin.TexC + i*texOffset;
		float3 neighborNormal = gNormalMap.SampleLevel(gsamPointClamp, tex, 0.0f).xyz;
        float  neighborDepth  = NdcDepthToViewDepth(gDepthMap.SampleLevel(gsamDepth, tex, 0.0f).r);

		if( dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f )
		{
            float weight = blurWeights[i + gBlurRadius];
			color += weight*gInputMap.SampleLevel(gsamPointClamp, tex, 0.0);
			totalWeight += weight;
		}
	}

    return color / totalWeight;
}
