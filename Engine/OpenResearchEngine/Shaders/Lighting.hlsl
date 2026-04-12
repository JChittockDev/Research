#ifndef NUM_LIGHTS
    #define NUM_LIGHTS 3
#endif

#define MaxLights 16

#include "LightingUtil.hlsl"

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gViewNormal : register(t2);
Texture2D gAlbedoSpec : register(t3);
Texture2D gReflection : register(t4);
Texture2D gMaterialId : register(t5);
Texture2D gTangent : register(t6);
Texture2D gAmbient : register(t7);
Texture2D gAmbientVerticalBlur : register(t8);
Texture2D gAmbientHorizontalBlur : register(t9);
Texture2D gSSS : register(t10);
Texture2D gRadiance[32] : register(t11);

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
    float Bump;
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
};

StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

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
    float3 Albedo = gAlbedoSpec.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    
    float3 SSS = gSSS.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    
    /// Greyscale radiance no albedo
    float3 Radiance = float3(0.0, 0.0, 0.0);
    /// Specular includes color from reflections
    float3 Specular = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        Radiance += gRadiance[i].Sample(gsamAnisotropicWrap, pin.TexC).rgb;
        Specular += gRadiance[NUM_LIGHTS + i].Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    }
    
    float4 Ambient = gAmbientLight * float4(Albedo, 1.0f);
    
    float3 Lighting = Ambient.rgb + (Albedo * Radiance) + (Albedo * SSS) + Specular;
    
    uint MatId = gMaterialId.Sample(gsamAnisotropicWrap, pin.TexC).r;
    
    MaterialData matData = gMaterialData[MatId];
    
    Lighting *= gAmbientHorizontalBlur.Sample(gsamAnisotropicWrap, pin.TexC).r;

    if (matData.Lit == 0)
    {
        Lighting = Albedo;
    }
    
	return float4(Lighting, 1.0f);
}