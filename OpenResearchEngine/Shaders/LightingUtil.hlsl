//***************************************************************************************
// LightingUtil.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Contains API for shader lighting.
//***************************************************************************************

#define MaxLights 16

#define PI 3.14159265359

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepth : register(s7);

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

struct LightingParameters
{
    float3 Color;
    float Reflectance;
    float Roughness;
    float Metalness;
    float3 ReflectedColor;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
}

// Fresnel reflectance using Schlick's approximation
// The Fresnel equation describes the ratio of surface reflection at different surface angles.
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    // Approximates how much light is reflected vs refracted
    // More reflectance at grazing angles (cosTheta near 0)
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX Normal Distribution Function (NDF)
// Models how microfacets are oriented relative to the surface normal.
float DistributionGGX(float3 N, float3 H, float roughness)
{
    // Convert roughness to alpha, representing surface microfacet slope
    float a = roughness * roughness;
    float a2 = a * a;

    // Calculate the cosine of the angle between the normal and halfway vector
    // This tells us how aligned a microfacet is with the halfway vector
    float NdotH = max(dot(N, H), 0.0); // Clamp to 0 to prevent invalid input
    float NdotH2 = NdotH * NdotH; // Square it for use in the GGX formula

    // Denominator of GGX formula:
    // Controls the shape of the specular highlight.
    // As N·H gets smaller (more grazing angle), the specular highlight fades.
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom; // Final normalization factor

    // Return the GGX normal distribution value
    // High when microfacets are aligned with H, low otherwise
    return a2 / denom;
}

// Geometry function for a single direction using Schlick-GGX approximation
// Approximates how much microfacets are shadowed or masked by each other.
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // k controls shadowing/masking. Derived from roughness.
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    // Denominator in the geometry term
    // Physically: how much of the microfacet is visible (not shadowed)
    float denom = NdotV * (1.0 - k) + k;

    // Return visibility factor for this direction
    return NdotV / denom;
}

// Geometry function combining both view and light directions
// Combines geometry factors from both the light and view directions.
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    // Dot product of normal with view and light vectors
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    // Calculate shadowing for both view and light
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    // Final geometry factor: how much of the surface is visible from both directions
    return ggx1 * ggx2;
}

// Cook-Torrance BRDF main function
// The full specular reflection model.
float3 CookTorranceBRDF(float3 N, float3 V, float3 L, float roughness, float3 F0)
{
    // Calculate halfway vector between view and light directions
    float3 H = normalize(V + L);

    // Microfacet distribution: how rough/smooth the surface is
    float D = DistributionGGX(N, H, roughness);

    // Geometry function: how microfacets are occluded
    float G = GeometrySmith(N, V, L, roughness);

    // Fresnel term: how much light reflects off microfacets
    // At glancing angles, reflection should be stronger as less light is refracted
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Dot products for shading scale factors
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Numerator of the Cook-Torrance BRDF
    // Combines D (distribution), G (geometry), and F (Fresnel)
    float3 numerator = D * G * F;

    // Denominator normalizes the BRDF based on geometry
    // Ensures energy conservation (prevents over-bright results)
    float denominator = 4.0 * NdotV * NdotL + 0.001; // epsilon prevents divide-by-zero

    // Final specular BRDF value
    return numerator / denominator;
}

// Main BRDF lighting function: combines diffuse + specular
// Simple energy-conserving diffuse reflection (only for non-metals).
float3 BRDF_Lighting(
    float3 N, // Surface normal
    float3 V, // View direction
    float3 L, // Light direction
    float3 albedo, // Base color of the material
    float roughness, // Surface microfacet roughness (0=smooth, 1=rough),
    float reflectance,
    float metalness, // Metalness (0 = dielectric, 1 = metal),
    float3 reflectedColor
)
{
    // Halfway vector between light and view
    float3 H = normalize(V + L);

    // Dot product of normal and light
    float NdotL = max(dot(N, L), 0.0);
    
    // Base reflectivity at normal incidence
    float cRef = saturate(reflectance * 0.04);
    float3 F0 = float3(cRef, cRef, cRef); // Dielectrics reflect ~4%
    F0 = lerp(F0, albedo, metalness); // Metals use albedo as F0

    // Compute specular term using Cook-Torrance BRDF
    float3 specular = CookTorranceBRDF(N, V, L, roughness, F0);

    // Diffuse coefficient: only for non-metals (metals have no diffuse)
    float3 kD = (1.0 - F0) * (1.0 - metalness);

    // Lambertian diffuse reflection
    float3 diffuse = kD * albedo / PI;
    
    float3 fresnel = FresnelSchlick(max(dot(N, H), 0.0), F0);

    // Chrome ball: mostly reflect, especially at low roughness
    float3 refMapSpecular = reflectedColor * fresnel;

    // Final color
    return (diffuse + specular) * NdotL + refMapSpecular * metalness;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(Light L, LightingParameters mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

    return BRDF_Lighting(normal, toEye, lightVec, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor) * L.Strength;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(Light L, LightingParameters mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if(d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    
    return BRDF_Lighting(normal, toEye, lightVec, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor) * L.Strength * att;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(Light L, LightingParameters mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if(d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);

    //float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.OuterConeAngle);
    //lightStrength *= spotFactor;
    
    // Scale by spotlight
    float spotAngle = dot(lightVec, -L.Direction);
    
    float epsilon = abs(L.InnerConeAngle - L.OuterConeAngle);
    float coneAttenuation = clamp((spotAngle - L.OuterConeAngle) / epsilon, 0.0f, 1.0f);
    
    return BRDF_Lighting(normal, toEye, lightVec, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor) * L.Strength * att * coneAttenuation;
}

float4 ComputeLighting(Light gLights[MaxLights], LightingParameters mat, float3 pos, float3 normal, float3 toEye, float shadowFactor[MaxLights])
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}

