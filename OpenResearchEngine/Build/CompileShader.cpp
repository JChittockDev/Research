#include "../EngineApp.h"

void EngineApp::CompileShaders ()
{
    std::string numDirectionalLights = std::to_string(dynamicLights.DirectionalLights.size());
    std::string numPointLights = std::to_string(dynamicLights.PointLights.size());
    std::string numSpotLights = std::to_string(dynamicLights.SpotLights.size());

    D3D_SHADER_MACRO directionalLightMacro;
    directionalLightMacro.Name = "NUM_DIR_LIGHTS";
    directionalLightMacro.Definition = numDirectionalLights.c_str();

    D3D_SHADER_MACRO pointLightMacro;
    pointLightMacro.Name = "NUM_POINT_LIGHTS";
    pointLightMacro.Definition = numPointLights.c_str();

    D3D_SHADER_MACRO spotLightMacro;
    spotLightMacro.Name = "NUM_SPOT_LIGHTS";
    spotLightMacro.Definition = numSpotLights.c_str();

    D3D_SHADER_MACRO skinningMacro;
    skinningMacro.Name = "SKINNED";
    skinningMacro.Definition = "1";

    D3D_SHADER_MACRO alphaMacro;
    alphaMacro.Name = "ALPHA_TEST";
    alphaMacro.Definition = "1";

    std::string factor = std::to_string(32768.0);
    D3D_SHADER_MACRO quantizeMacro;
    quantizeMacro.Name = "QUANTIZE";
    quantizeMacro.Definition = factor.c_str();

    D3D_SHADER_MACRO null;
    null.Name = NULL;
    null.Definition = NULL;

    const D3D_SHADER_MACRO opaqueDefines[4] = {directionalLightMacro, pointLightMacro, spotLightMacro, null};
    const D3D_SHADER_MACRO alphaTestDefines[2] = {alphaMacro, null};
    const D3D_SHADER_MACRO skinnedDefines[2] = {skinningMacro, null};
    const D3D_SHADER_MACRO clothDefines[2] = { quantizeMacro, null };

    std::string shaderPath = GetFullPath("Shaders/Lighting.hlsl");
    std::wstring wLightingShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["LightingVS"] = d3dUtil::CompileShader(wLightingShaderPath, nullptr, "VS", "vs_5_1");
    mShaders["LightingPS"] = d3dUtil::CompileShader(wLightingShaderPath, opaqueDefines, "PS", "ps_5_1");

    shaderPath = GetFullPath("Shaders/GBuffer.hlsl");
    std::wstring wGbufferShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["GBufferVS"] = d3dUtil::CompileShader(wGbufferShaderPath, nullptr, "VS", "vs_5_1");
    mShaders["GBufferPS"] = d3dUtil::CompileShader(wGbufferShaderPath, opaqueDefines, "PS", "ps_5_1");

    shaderPath = GetFullPath("Shaders/Shadows.hlsl");
    std::wstring wShadowShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["shadowVS"] = d3dUtil::CompileShader(wShadowShaderPath, nullptr, "VS", "vs_5_1");
    mShaders["shadowOpaquePS"] = d3dUtil::CompileShader(wShadowShaderPath, nullptr, "PS", "ps_5_1");
    mShaders["shadowAlphaTestedPS"] = d3dUtil::CompileShader(wShadowShaderPath, alphaTestDefines, "PS", "ps_5_1");

    shaderPath = GetFullPath("Shaders/ComputeBlendshapes.hlsl");
    std::wstring wBlendshapeShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["blendCS"] = d3dUtil::CompileShader(wBlendshapeShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeSkinning.hlsl");
    std::wstring wSkinningShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["skinnedCS"] = d3dUtil::CompileShader(wSkinningShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeTriangleNormals.hlsl");
    std::wstring wTriangleNormalsShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["triangleNormalCS"] = d3dUtil::CompileShader(wTriangleNormalsShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeVertexNormals.hlsl");
    std::wstring wVertexNormalsShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["vertexNormalCS"] = d3dUtil::CompileShader(wVertexNormalsShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeMeshTransfer.hlsl");
    std::wstring wMeshTransferShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["meshTransferCS"] = d3dUtil::CompileShader(wMeshTransferShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeSimMeshTransfer.hlsl");
    std::wstring wSimMeshTransferShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["simMeshTransferCS"] = d3dUtil::CompileShader(wSimMeshTransferShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeTension.hlsl");
    std::wstring wTensionShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["tensionCS"] = d3dUtil::CompileShader(wTensionShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeForce.hlsl");
    std::wstring wForceShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["forceCS"] = d3dUtil::CompileShader(wForceShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputePreSolve.hlsl");
    std::wstring wPreSolveShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["preSolveCS"] = d3dUtil::CompileShader(wPreSolveShaderPath, nullptr, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputeConstraintSolve.hlsl");
    std::wstring wConstraintSolveShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["constraintSolveCS"] = d3dUtil::CompileShader(wConstraintSolveShaderPath, clothDefines, "CS", "cs_5_1");

    shaderPath = GetFullPath("Shaders/ComputePostSolve.hlsl");
    std::wstring wPostSolveShaderPath(shaderPath.begin(), shaderPath.end());
    mShaders["postSolveCS"] = d3dUtil::CompileShader(wPostSolveShaderPath, clothDefines, "CS", "cs_5_1");

    mInputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };
}