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

    mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", opaqueDefines, "PS", "ps_5_1");

    mShaders["shadowVS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["shadowOpaquePS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");
    mShaders["shadowAlphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", alphaTestDefines, "PS", "ps_5_1");

    mShaders["debugVS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["debugPS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["drawNormalsVS"] = d3dUtil::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["drawNormalsPS"] = d3dUtil::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["ssaoVS"] = d3dUtil::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["ssaoPS"] = d3dUtil::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["ssaoBlurVS"] = d3dUtil::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["ssaoBlurPS"] = d3dUtil::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["skinnedCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeSkinning.hlsl", nullptr, "CS", "cs_5_1");
    mShaders["triangleNormalCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeTriangleNormals.hlsl", nullptr, "CS", "cs_5_1");
    mShaders["vertexNormalCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeVertexNormals.hlsl", nullptr, "CS", "cs_5_1");

    mShaders["meshTransferCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeMeshTransfer.hlsl", nullptr, "CS", "cs_5_1");
    mShaders["simMeshTransferCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeSimMeshTransfer.hlsl", nullptr, "CS", "cs_5_1");

    mShaders["forceCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeForce.hlsl", nullptr, "CS", "cs_5_1");
    mShaders["preSolveCS"] = d3dUtil::CompileShader(L"Shaders\\ComputePreSolve.hlsl", nullptr, "CS", "cs_5_1");

    mShaders["stretchConstraintSolveCS"] = d3dUtil::CompileShader(L"Shaders\\ComputeStretchConstraintSolve.hlsl", nullptr, "CS", "cs_5_1");

    mInputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };
}