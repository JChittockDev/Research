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
Texture2D gViewNormal : register(t2);
Texture2D gAlbedoSpec : register(t3);
Texture2D gReflection : register(t4);
Texture2D gMaterialId : register(t5);
Texture2D gAmbient : register(t6);
Texture2D gAmbientVerticalBlur : register(t7);
Texture2D gAmbientHorizontalBlur : register(t8);
Texture2D gShadowMap[16] : register(t9);

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
    float4 Color;
    float Reflectance;
    float Roughness;
    float Metalness;
    int Lit;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint NormalMapIndex;
    uint RoughnessMapIndex;
    uint MetalnessMapIndex;
    uint SpecularMapIndex;
    uint HeightsMapIndex;
    uint OpacityMapIndex;
    uint OcclusionMapIndex;
    uint RefractionMapIndex;
    uint EmissiveMapIndex;
    uint SubsurfaceMapIndex;
    uint ReflectionMapIndex;
    uint Padding0; // to align total size to 16 bytes
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
    
    uint MatId = gMaterialId.Sample(gsamAnisotropicWrap, pin.TexC).r;
    
    MaterialData matData = gMaterialData[MatId];
    
    float3 toEyeW = normalize(gEyePosW - FragPos);
    
        // Ambient lighting
    float4 ambient = gAmbientLight * float4(Albedo, 1.0f);

    // Shadowing (if needed)
    float shadowFactor[MaxLights];
    for (int i = 0; i < (NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS); ++i)
    {
        shadowFactor[i] = CalcShadowFactor(mul(float4(FragPos, 1.0), gShadowTransform[i]), gShadowMap[i]);
    }
    
    float3 R = reflect(-toEyeW, Normal); // View-reflected vector (world space)
    float3 reflectedColor = gReflection.Sample(gsamAnisotropicWrap, R.xy);
    
    // Material for lighting calculation
    LightingParameters mat = { Albedo, matData.Reflectance * Specular, matData.Roughness, matData.Metalness, reflectedColor };

    float4 directLight = ComputeLighting(gLights, mat, FragPos, Normal, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
    litColor *= gAmbientHorizontalBlur.Sample(gsamAnisotropicWrap, pin.TexC).r;

    // Alpha = 1 for deferred (unless you handle transparency separately)
    litColor.a = 1.0f;
    
    if (matData.Lit == 0)
    {
        litColor = float4(Albedo, 1.0f);

    }
    
    return litColor;
}