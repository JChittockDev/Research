cbuffer cbSsao : register(b0)
{
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gProjTex;
	float4   gOffsetVectors[14];

    float    gGiSampleRadius;
    float    gGiFalloffScale;
    float pad1;
    float pad2;
};
 
Texture2D gLighting : register(t0);
Texture2D gNormalMap    : register(t1);
Texture2D gRandomVecMap : register(t2);
Texture2D gDepthMap : register(t3);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepth : register(s7);

static const int gSampleCount = 14;
 
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
    float3 PosV : POSITION;
	float2 TexC : TEXCOORD0;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;

    vout.TexC = gTexCoords[vid];
    vout.PosH = float4(2.0f*vout.TexC.x - 1.0f, 1.0f - 2.0f*vout.TexC.y, 0.0f, 1.0f);
    float4 ph = mul(vout.PosH, gInvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}

float NdcDepthToViewDepth(float z_ndc)
{
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}
 
float4 PS(VertexOut pin) : SV_Target
{
    // Sample normal and reconstruct position in view space
    float3 n = gNormalMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz;
    float pz = gDepthMap.SampleLevel(gsamDepth, pin.TexC, 0.0f).r;
    pz = NdcDepthToViewDepth(pz);
    float3 p = (pz / pin.PosV.z) * pin.PosV;

    // Get random vector to rotate sampling pattern
    float3 randVec = 2.0f * gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f * pin.TexC, 0.0f).rgb - 1.0f;

    float3 bounceLightSum = float3(0.0f, 0.0, 0.0);

    for (int i = 0; i < gSampleCount; ++i)
    {
        float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
        float flip = sign(dot(offset, n));
        float3 q = p + flip * gGiSampleRadius * offset;

        float4 projQ = mul(float4(q, 1.0f), gProjTex);
        projQ /= projQ.w;

        // Sample the output from the screen space lighting pass
        float3 light = gLighting.SampleLevel(gsamPointClamp, projQ.xy, 0.0f).xyz;
            
        if (light.r > 0.0 || light.g > 0.0 || light.b > 0.0)
        {
            float rz = gDepthMap.SampleLevel(gsamDepth, projQ.xy, 0.0f).r;
            rz = NdcDepthToViewDepth(rz);
            float3 r = (rz / q.z) * q;

            float distZ = p.z - r.z;
            float clampedDistanceZ = clamp(distZ, 0.0, gGiFalloffScale);
            float bouncedLight = exp(-pow(clampedDistanceZ / gGiFalloffScale, 2.0)) * light;

            // Sample normal at projQ.xy
            float3 nQ = gNormalMap.SampleLevel(gsamPointClamp, projQ.xy, 0.0f).xyz;

            //// Compute dot product between n (at pin.TexC) and inverse of nQ
            //float attenuation = dot(n, -nQ);
            //attenuation = saturate(attenuation); // Clamp to [0, 1]
            
            bouncedLight *= max(dot(n, normalize(r - p)), 0.0f);
            bounceLightSum += light * bouncedLight;
        }
    }

    bounceLightSum /= gSampleCount;

    return float4(bounceLightSum, 1.0);
}
