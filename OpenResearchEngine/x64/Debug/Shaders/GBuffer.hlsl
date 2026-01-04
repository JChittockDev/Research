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

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW, float bump)
{
    // BC5: .r = x, .g = y, reconstruct z
    float2 encXY = 2.0f * normalMapSample.rg - 1.0f;
    float z = sqrt(saturate(1.0f - dot(encXY, encXY)));
    float3 normalT = float3(encXY, z);

    // Create orthonormal TBN matrix
    float3 N = normalize(unitNormalW);
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);

    // Transform to world space
    float3 bumpedNormalW = mul(normalT, TBN);

    // Blend between original normal and bumped normal using bump intensity
    bumpedNormalW = normalize(lerp(N, bumpedNormalW, bump));

    return bumpedNormalW;
}

Texture2D gTextureMaps[48] : register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepth : register(s7);

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
    float4 ViewNormal : SV_Target2;
    float4 AlbedoSpec : SV_Target3;
    float4 Reflection : SV_Target4;
    float4 MatId : SV_Target5;
    float4 Tangent : SV_Target6;
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
    float4 diffuseAlbedo = matData.Color;
	
    // Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMaps[matData.DiffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord);
    float specular = gTextureMaps[matData.SpecularMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord).x;
    
    float4 normalMapSample = gTextureMaps[matData.NormalMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.Normal, pin.Tangent, matData.Bump);
    
    float4 reflection = gTextureMaps[matData.ReflectionMapIndex].Sample(gsamAnisotropicWrap, pin.TexCoord);

    pout.Position = float4(pin.WorldPosition, 1.0);
    pout.Normal = float4(normalize(bumpedNormalW), 1.0);
    pout.ViewNormal = mul(float4(normalize(bumpedNormalW), 1.0), gView);
    pout.AlbedoSpec = float4(diffuseAlbedo.xyz, specular);
    pout.Reflection = reflection;
    pout.MatId = float4(gMaterialIndex, 0.0, 0.0, 1.0);
    pout.Tangent = float4(pin.Tangent, 1.0);
    
    return pout;
}