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
    uint LightType;
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
void BRDF_Lighting(
    float3 N, // Surface normal
    float3 V, // View direction
    float3 L, // Light direction
    float3 albedo, // Base color of the material
    float roughness, // Surface microfacet roughness (0=smooth, 1=rough)
    float reflectance,
    float metalness, // Metalness (0 = dielectric, 1 = metal)
    float3 reflectedColor,
    out float3 diffuseReflectance, // without albedo
    out float3 specularReflectance
)
{
    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    
    float cRef = saturate(0.16 * reflectance * reflectance);
    float3 F0 = float3(cRef, cRef, cRef);
    F0 = lerp(F0, albedo, metalness);

    float3 specular = CookTorranceBRDF(N, V, L, roughness, F0);
    float3 kD = (1.0 - F0) * (1.0 - metalness);

    diffuseReflectance = kD * (albedo / PI) * NdotL;

    float3 fresnel = FresnelSchlick(max(dot(N, H), 0.0), F0);
    float3 refMapSpecular = reflectedColor * fresnel;

    specularReflectance = (specular * NdotL) + (refMapSpecular * metalness);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Light L, LightingParameters mat, float3 normal, float3 toEye, out float3 diffuseReflectance, out float3 specularReflectance)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;
    
    BRDF_Lighting(normal, toEye, lightVec, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor, diffuseReflectance, specularReflectance);
    
    diffuseReflectance *= L.Strength;
    specularReflectance *= L.Strength;

}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
void ComputePointLight(Light L, LightingParameters mat, float3 pos, float3 normal, float3 toEye, out float3 diffuseReflectance, out float3 specularReflectance)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if(d > L.FalloffEnd)
        return;

    // Normalize the light vector.
    lightVec /= d;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    
    BRDF_Lighting(normal, toEye, lightVec, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor, diffuseReflectance, specularReflectance);
    
    diffuseReflectance *= L.Strength * att;
    specularReflectance *= L.Strength * att;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Light L, LightingParameters mat, float3 pos, float3 normal, float3 toEye, out float3 diffuseReflectance, out float3 specularReflectance)
{
    // Initialize outputs
    diffuseReflectance = float3(0.0f, 0.0f, 0.0f);
    specularReflectance = float3(0.0f, 0.0f, 0.0f);
    
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test - early exit if outside light range
    if (d > L.FalloffEnd)
        return;

    // Normalize the light vector.
    float3 lightVecNorm = lightVec / d;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);

    // Calculate spotlight cone attenuation
    // The angle between the light direction and the vector from light to surface
    float cosAngle = dot(normalize(L.Direction), -lightVecNorm);
    
    // Convert cone angles from degrees to cosine values for comparison
    float cosInnerCone = cos(radians(L.InnerConeAngle));
    float cosOuterCone = cos(radians(L.OuterConeAngle));
    
    // Calculate cone attenuation with smooth falloff between inner and outer cone
    float coneAttenuation = 0.0f;
    if (cosAngle > cosOuterCone)
    {
        if (cosAngle > cosInnerCone)
        {
            // Inside inner cone - full intensity
            coneAttenuation = 1.0f;
        }
        else
        {
            // Between inner and outer cone - smooth falloff
            coneAttenuation = smoothstep(cosOuterCone, cosInnerCone, cosAngle);
        }
    }
    // Outside outer cone - no light contribution (already 0.0f)
    
    // Only calculate BRDF if there's actual light contribution
    if (coneAttenuation > 0.0f && att > 0.0f)
    {
        BRDF_Lighting(normal, toEye, lightVecNorm, mat.Color, mat.Roughness, mat.Reflectance, mat.Metalness, mat.ReflectedColor, diffuseReflectance, specularReflectance);
        
        // Apply all attenuation factors
        float totalAttenuation = att * coneAttenuation;
        diffuseReflectance *= L.Strength * totalAttenuation;
        specularReflectance *= L.Strength * totalAttenuation;
    }
}

void ComputeLighting(Light gLight, LightingParameters mat, float3 pos, float3 normal, float3 toEye, out float3 diffuseReflectance, out float3 specularReflectance)
{
    
    if (gLight.LightType == 0)
    {
        ComputeDirectionalLight(gLight, mat, normal, toEye, diffuseReflectance, specularReflectance);
    }
    else if (gLight.LightType == 1)
    {
        ComputePointLight(gLight, mat, pos, normal, toEye, diffuseReflectance, specularReflectance);
    }
    else
    {
        ComputeSpotLight(gLight, mat, pos, normal, toEye, diffuseReflectance, specularReflectance);
    }

}

