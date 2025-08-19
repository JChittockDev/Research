#include "LightingUtil.hlsl"

#ifndef NUM_LIGHTS
#define NUM_LIGHTS 3
#endif

#define PI 3.14159265359f

cbuffer cbSss : register(b0)
{
    float4x4 gProj; // 16
    float4x4 gInvProj; // 16
    float4x4 gViewProj; // 16
    float4x4 gProjTex; // 16
    float3 gEyePosW; // 12
    float gBlend; // 4
    float gThickness; // 4
    float3 gTransmissionColor; // 12
    float3 gScatteringProfile; // 12
    uint gSampleCount; // 4
    float gScale;
    float gPadding1;
    float gPadding2;
    float gPadding3;
    Light gLights[MaxLights];
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

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D gTangent : register(t3);
Texture2D gMaterialId : register(t4);
Texture2D gRadiance[16] : register(t5); // lighting without albedo

StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

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

// Burley diffusion profile implementation
// Based on "Approximate Reflectance Profiles for Efficient Subsurface Scattering" (Burley 2015)
float3 BurleyDiffusionProfile(float r, float3 scatterDistance)
{
    float3 result = float3(0, 0, 0);
    
    // Avoid division by zero
    scatterDistance = max(scatterDistance, float3(0.001, 0.001, 0.001));
    
    for (int i = 0; i < 3; i++)
    {
        float d = scatterDistance[i];
        float rOverD = r / d;
        
        // Burley's normalized diffusion profile
        // R(r) = (exp(-r/d) + exp(-r/(3*d))) / (8*pi*d*r)
        float term1 = exp(-rOverD);
        float term2 = exp(-rOverD / 3.0f);
        
        result[i] = (term1 + term2) / (8.0f * PI * d * max(r, 0.001f));
    }
    
    return result;
}

// Sample a point on a disk using concentric mapping
float2 SampleConcentricDisk(float2 u)
{
    // Map uniform random numbers to [-1,1]^2
    float2 uOffset = 2.0f * u - float2(1, 1);
    
    // Handle degeneracy at the origin
    if (uOffset.x == 0 && uOffset.y == 0)
        return float2(0, 0);
    
    // Apply concentric mapping to point
    float theta, r;
    if (abs(uOffset.x) > abs(uOffset.y))
    {
        r = uOffset.x;
        theta = (PI / 4.0f) * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = (PI / 2.0f) - (PI / 4.0f) * (uOffset.x / uOffset.y);
    }
    
    return r * float2(cos(theta), sin(theta));
}

// Importance sample the Burley diffusion profile
float SampleBurleyDiffusionProfile(float u, float scatterDistance)
{
    // This uses the CDF inversion method for the Burley profile
    // Simplified approximation for real-time use
    float maxRadius = scatterDistance * 10.0f; // Practical cutoff
    
    // Use a simple power function as approximation for sampling
    // This isn't the exact inverse CDF but works well in practice
    return maxRadius * pow(u, 0.25f);
}


// Build orthonormal basis from normal
void BuildOrthonormalBasis(float3 n, out float3 t, out float3 b)
{
    if (abs(n.z) > 0.9f)
    {
        t = float3(1, 0, 0);
    }
    else
    {
        t = float3(0, 0, 1);
    }
    
    t = normalize(t - dot(t, n) * n);
    b = cross(n, t);
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
    uint MatId = gMaterialId.Sample(gsamAnisotropicWrap, pin.TexC).r;
    MaterialData matData = gMaterialData[MatId];
    
    // This means that it is an SSS material. If it is not, it shouldn't render. 
    // This shader should also check to make sure that it is not sampling from any non SSS objects.
    if (matData.Lit != 2)
    {
        // Not an SSS material, return transparent/black or discard
        discard; // Alternative: return float4(0, 0, 0, 0);
    }
    
    // Load center pixel data
    float3 centerPos = gPosition.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz;
    float3 centerNormal = normalize(gNormal.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz);
    float3 centerTangent = gTangent.SampleLevel(gsamPointClamp, pin.TexC, 0).xyz;
    
    // Build tangent frame
    float3 tangent, bitangent;
    if (length(centerTangent) > 0.001f)
    {
        tangent = normalize(centerTangent);
        bitangent = cross(centerNormal, tangent);
    }
    else
    {
        BuildOrthonormalBasis(centerNormal, tangent, bitangent);
    }
    
    // Material parameters
    float3 scatterDistance = gScatteringProfile * gScale;
    scatterDistance = max(scatterDistance, float3(0.001, 0.001, 0.001));
    
    // Accumulate SSS contribution
    float3 sssAccum = float3(0, 0, 0);
    uint sampleCount = clamp(gSampleCount, 1, 64);
    
    for (uint lightIndex = 0; lightIndex < min(NUM_LIGHTS, 16); ++lightIndex)
    {
        float3 lightVector = GetLightVector(gLights[lightIndex], centerPos);
        if (length(lightVector) < 0.001f)
            continue;
        
        float3 lightDir = normalize(lightVector);
        
        // Use rejection sampling to maintain consistent sample count
        uint attempts = 0;
        const uint maxAttempts = sampleCount * 4; // Prevent infinite loops
        float3 lightSSSAccum = float3(0, 0, 0);
        uint validSamples = 0;
        
        while (validSamples < sampleCount && attempts < maxAttempts)
        {
            // Generate random numbers for this attempt
            float2 pixelSeed = pin.TexC * 1000.0 + float2(attempts + lightIndex * maxAttempts, (attempts + lightIndex * maxAttempts) * 2);
            float2 rand2 = hash2(pixelSeed);
            float rand1 = hash(pixelSeed + float2(123.45, 678.90));
            
            attempts++;
            
            // Importance sample the diffusion profile to get radius
            float avgScatterDistance = (scatterDistance.r + scatterDistance.g + scatterDistance.b) / 3.0f;
            float radius = SampleBurleyDiffusionProfile(rand1, avgScatterDistance);
            
            // Sample point on disk
            float2 diskSample = SampleConcentricDisk(rand2) * radius;
            
            // Offset in world space using tangent frame
            float3 sampleOffset = diskSample.x * tangent + diskSample.y * bitangent;
            float3 sampleWorldPos = centerPos + sampleOffset;
            
            // Project sample to screen space
            float4 clipPos = mul(float4(sampleWorldPos, 1.0), gViewProj);
            if (abs(clipPos.w) < 0.001f)
                continue;
            
            float2 ndc = clipPos.xy / clipPos.w;
            float2 sampleUV = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);
            
            // Check screen bounds
            if (sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y > 1)
                continue;
            
            // Sample radiance and normal at this position
            float3 sampleRadiance = gRadiance[lightIndex].SampleLevel(gsamLinearClamp, sampleUV, 0).rgb;
            float3 sampleNormal = normalize(gNormal.SampleLevel(gsamLinearClamp, sampleUV, 0).xyz);
            
            // Skip invalid samples
            if (all(sampleRadiance <= 0.0f) || length(sampleNormal) < 0.001f)
                continue;
            
            // Additional check: Make sure we're not sampling from non-SSS materials
            uint sampleMatId = gMaterialId.SampleLevel(gsamPointClamp, sampleUV, 0).r;
            MaterialData sampleMatData = gMaterialData[sampleMatId];
            if (sampleMatData.Lit != 2)
                continue; // Skip non-SSS materials in sampling
            
            // Calculate actual distance from center
            float actualRadius = length(sampleOffset);
            if (actualRadius < 0.001f)
                continue;
            
            // Evaluate Burley diffusion profile
            float3 profileWeight = BurleyDiffusionProfile(actualRadius, scatterDistance);
            
            // Lambert's cosine law for the sample
            float cosThetaI = saturate(dot(sampleNormal, lightDir));
            
            // Calculate BSSRDF contribution
            // BSSRDF = albedo * profile * cosine / pi
            float3 bssrdf = profileWeight * cosThetaI / PI;
            
            // Accumulate contribution
            lightSSSAccum += sampleRadiance * bssrdf;
            validSamples++;
        }
        
        // Average over valid samples
        if (validSamples > 0)
        {
            lightSSSAccum /= float(validSamples);
            sssAccum += lightSSSAccum;
        }
    }
    
    // Apply transmission color and combine with albedo
    float3 finalColor = sssAccum * gTransmissionColor;
    
    return float4(saturate(finalColor), 1.0f);
}