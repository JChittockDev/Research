#include "RadianceUtil.hlsl"

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gViewNormal : register(t2);
Texture2D gAlbedoSpec : register(t3);
Texture2D gReflection : register(t4);
Texture2D gMaterialId : register(t5);
Texture2D gShadowMap : register(t6);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    uint gObjPad2;
};

cbuffer cbRadiance : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gShadowTransform;
    float3 gEyePosW;
    float uPadding1;
    Light gLight;
    uint gLightType;
    float uPadding2;
    float uPadding3;
    float uPadding4;
    
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

struct PixelOut
{
    float4 DiffuseReflect: SV_Target0;
    float4 SpecularReflect : SV_Target1;
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


PixelOut PS(VertexOut pin)
{   
    PixelOut pout;
    float3 FragPos = gPosition.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float3 Normal = gNormal.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float3 Albedo = gAlbedoSpec.Sample(gsamAnisotropicWrap, pin.TexC).rgb;
    float Specular = gAlbedoSpec.Sample(gsamAnisotropicWrap, pin.TexC).a;
    
    uint MatId = gMaterialId.Sample(gsamAnisotropicWrap, pin.TexC).r;
    
    MaterialData matData = gMaterialData[MatId];
    
    float3 toEyeW = normalize(gEyePosW - FragPos);
    
    float3 R = reflect(-toEyeW, Normal); // View-reflected vector (world space)
    float3 reflectedColor = gReflection.Sample(gsamAnisotropicWrap, R.xy);

    // Shadowing (if needed)
    float shadowFactor = CalcShadowFactor (mul(float4(FragPos, 1.0), gShadowTransform), gShadowMap);

    float3 diffuseReflectance = 0.0;
    float3 specularReflectance = 0.0;
    
    // Material for lighting calculation
    // No albedo just radiance
    LightingParameters mat = { float3(1.0, 1.0, 1.0) , matData.Reflectance * Specular, matData.Roughness, matData.Metalness, reflectedColor };
    
    ComputeLighting(gLight, mat, FragPos, Normal, toEyeW, diffuseReflectance, specularReflectance);
    
    diffuseReflectance *= shadowFactor;
    specularReflectance *= shadowFactor;

    pout.DiffuseReflect = float4(diffuseReflectance, 1.0);
    pout.SpecularReflect = float4(specularReflectance, 1.0);
    return pout;
}