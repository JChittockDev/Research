#include "SubsurfaceMaterial.hlsli"
#include "SubsurfaceScattering.hlsli"

cbuffer cbSss : register(b0)
{
    float4x4 gProj; // 16
    float4x4 gInvProj; // 16
    float4x4 gViewProj; // 16
    float4x4 gProjTex; // 16
    float3 gEyePosW; // 12
    float gAnisotropy; // 4
    float gThickness; // 4
    float3 gTransmissionColor; // 12
    float3 gScatteringProfile; // 12
    uint gSampleCount; // 4
    float gScale;
    float gPadding1;
    float gPadding2;
    float gPadding3;
};

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D gIrradiance : register(t3); // lighting without albedo

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
    float3 PosV : POSITION;
	float2 TexC : TEXCOORD0;
};

// Simple hash function for generating pseudo-random numbers
float hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453123);
}

float2 hash2(float2 p)
{
    return frac(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

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
    // Load center pixel data
    float3 centerPos = gPosition.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz;
    float3 centerNormal = normalize(gNormal.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz);
    float3 albedo = gAlbedo.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz;
    
    // Setup material data
    RTXCR_SubsurfaceMaterialData matData;
    matData.transmissionColor = gTransmissionColor;
    matData.scatteringColor = gScatteringProfile;
    matData.scale = gScale;
    matData.g = gAnisotropy;

    // Setup subsurface interaction for center pixel
    RTXCR_SubsurfaceInteraction interaction;
    interaction.centerPosition = centerPos;
    interaction.normal = centerNormal;
    
    // Build tangent frame
    float3 up = abs(centerNormal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    interaction.tangent = normalize(cross(up, centerNormal));
    interaction.biTangent = cross(centerNormal, interaction.tangent);

    // Sample multiple points using RTXCR library
    float3 sssAccum = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < gSampleCount; ++i)
    {
        // Generate random numbers for this sample
        float2 pixelSeed = pin.TexC * 1000.0 + float2(i, i * 2);
        float2 rand2 = hash2(pixelSeed);
        
        // Let RTXCR generate the sample position and weight
        RTXCR_SubsurfaceSample subsurfaceSample;
        
        // This function generates the sample position in world space
        // and calculates the appropriate diffusion profile weight
        RTXCR_EvalBurleyDiffusionProfile(matData, interaction, gThickness, true, rand2, subsurfaceSample);
        
        // Project the generated sample position to screen space
        float4 clipPos = mul(float4(subsurfaceSample.samplePosition, 1.0), gViewProj);
        float2 ndc = clipPos.xy / clipPos.w;
        float2 sampleUV = ndc * 0.5 + 0.5;
        
        // Check if sample is within screen bounds
        if (sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y > 1)
            continue;
            
        // Sample irradiance at the generated position
        float3 sampleIrradiance = gIrradiance.SampleLevel(gsamLinearClamp, sampleUV, 0);
        float3 sampleNormal = normalize(gNormal.SampleLevel(gsamLinearClamp, sampleUV, 0));
        
        // Calculate light direction for BSSRDF evaluation
        float3 lightVector = normalize(subsurfaceSample.samplePosition - centerPos);
        float cosThetaI = saturate(dot(sampleNormal, lightVector));
        
        // Evaluate BSSRDF using the sample data
        float3 bssrdfContribution = RTXCR_EvalBssrdf(subsurfaceSample, sampleIrradiance, cosThetaI);
        
        // Accumulate contribution weighted by the diffusion profile
        sssAccum += bssrdfContribution * subsurfaceSample.bssrdfWeight;
    }
    
    // Average the samples
    sssAccum /= float(gSampleCount);
    
    // Combine SSS with albedo
    float3 finalColor = albedo * sssAccum;
    
    return float4(finalColor, 1.0);
}
