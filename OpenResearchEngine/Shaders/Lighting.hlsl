#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 0
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 3
#endif

#define MaxLights 16

#include "LightingUtil.hlsl"

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedoSpec : register(t2);
Texture2D gShadowMap[16] : register(t3);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    uint gObjPad2;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gViewProjTex;
    float4x4 gShadowTransform[MaxLights];
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    Light gLights[MaxLights];
};

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint NormalMapIndex;
    uint RoughnessMapIndex;
    uint MetalnessMapIndex;
    uint SpecularMapIndex;
    uint HeightMapIndex;
    uint OpacityMapIndex;
    uint OcclusionMapIndex;
    uint RefractionMapIndex;
    uint EmissiveMapIndex;
    uint SubsurfaceMapIndex;
    uint MatPad1;
};

StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

float CalcShadowFactor(float4 shadowPosH, Texture2D shadowMap)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}

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
    vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
    float4 ph = mul(vout.PosH, gInvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{   
    float3 FragPos = gPosition.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float3 Normal = gNormal.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float3 Albedo = gAlbedoSpec.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float Specular = gAlbedoSpec.Sample(gsamAnisotropicWrap, pin.TexC).a;
    
    MaterialData matData = gMaterialData[gMaterialIndex];
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    
    float3 toEyeW = normalize(gEyePosW - FragPos);
    
        // Ambient lighting
    float4 ambient = gAmbientLight * float4(Albedo, 1.0f);

    // Shadowing (if needed)
    float shadowFactor[MaxLights];
    for (int i = 0; i < (NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS); ++i)
    {
        shadowFactor[i] = CalcShadowFactor(mul(float4(FragPos, 1.0), gShadowTransform[i]), gShadowMap[i]);
    }

    // Material for lighting calculation
    const float shininess = (1.0f - roughness) * Specular;
    Material mat = { float4(Albedo, 1.0f), fresnelR0, shininess };

    float4 directLight = ComputeLighting(gLights, mat, FragPos, Normal, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Reflections
    float3 reflected = reflect(-toEyeW, Normal);
    float4 reflectionColor = float4(1.0, 1.0, 1.0, 1.0);//gCubeMap.Sample(gsamLinearWrap, reflected);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, Normal, reflected);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;

    // Alpha = 1 for deferred (unless you handle transparency separately)
    litColor.a = 1.0f;
    
    return litColor;
}