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

struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction; // directional/spot light only
    float FalloffEnd; // point/spot light only
    float3 Position; // point light only
    float InnerConeAngle; // spot light only
    float OuterConeAngle; // spot light only
    float Pad1;
    float Pad2;
    float Pad3;
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
    uint HeightsMapIndex;
    uint OpacityMapIndex;
    uint OcclusionMapIndex;
    uint RefractionMapIndex;
    uint EmissiveMapIndex;
    uint SubsurfaceMapIndex;
    uint MatPad1;
};

Texture2D gTextureMaps[48] : register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

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

struct VertexIn
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
};

struct PixelOut
{
    float4 Position : SV_Target0;
    float4 Normal : SV_Target1;
    float4 AlbedoSpec : SV_Target2;
};

VertexOut VS(VertexIn vin, uint vertexID : SV_VertexID)
{
    VertexOut vout = (VertexOut) 0.0f;
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 posW = mul(float4(vin.Position, 1.0f), gWorld);
    vout.WorldPosition = posW.xyz;
    vout.Normal = mul(vin.Normal, (float3x3) gWorld);
    vout.Tangent = mul(vin.Tangent.xyz, (float3x3) gWorld);
    vout.Position = mul(posW, gViewProj);
    float4 texC = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
    vout.TexCoord = mul(texC, matData.MatTransform).xy;
    
    return vout;
}

PixelOut PS(VertexOut pin)
{
    PixelOut pout;
	// Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    uint specMapIndex = matData.SpecularMapIndex;
	
    // Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord);
    float specular = gTextureMaps[specMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord).x;

    pout.Position = pin.Position;
    pout.Normal = float4(normalize(pin.Normal), 1.0);
    pout.AlbedoSpec = float4(diffuseAlbedo.xyz, specular);
    
    return pout;
}