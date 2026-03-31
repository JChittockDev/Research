#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Render/Resources/FrameResource.h"
#include "../Render/Resources/Ssao.h"
#include "../Render/Resources/Ssgi.h"
#include "../Render/Resources/SSS.h"
#include "../Utilities/GameTimer.h"
#include <imgui.h>
#include <DirectXMath.h>

void UpdateScreenSpaceCB(const GameTimer& gt, SceneState& state, const Ssao* ssao, const Ssgi* ssgi, const SSS* sss, FrameResource* fr)
{
    SsaoConstants ssaoCB;
    SsaoBlurConstants ssaoVerticalBlurCB;
    SsaoBlurConstants ssaoHorizontalBlurCB;

    SsgiConstants ssgiCB;
    SsgiBlurConstants ssgiVerticalBlurCB;
    SsgiBlurConstants ssgiHorizontalBlurCB;

    // Recover original (non-transposed) projection matrix from stored transposed value
    DirectX::XMMATRIX P = XMMatrixTranspose(XMLoadFloat4x4(&state.mainPassCB.Proj));

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    ssaoCB.Proj = state.mainPassCB.Proj;
    ssaoCB.InvProj = state.mainPassCB.InvProj;
    ssaoVerticalBlurCB.Proj = state.mainPassCB.Proj;
    XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

    ssao->GetOffsetVectors(ssaoCB.OffsetVectors);

    auto blurWeights = ssao->CalcGaussWeights(2.5f);
    ssaoVerticalBlurCB.BlurWeights[0] = DirectX::XMFLOAT4(&blurWeights[0]);
    ssaoVerticalBlurCB.BlurWeights[1] = DirectX::XMFLOAT4(&blurWeights[4]);
    ssaoVerticalBlurCB.BlurWeights[2] = DirectX::XMFLOAT4(&blurWeights[8]);
    ssaoVerticalBlurCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / ssao->SsaoWidth(), 1.0f / ssao->SsaoHeight());
    ssaoVerticalBlurCB.BlurRadius = 5;

    static float occlusionRadius = 1.011f;
    static float occlusionFadeStart = 1.2f;
    static float occlusionFadeEnd = 4.3f;
    static float surfaceEpsilon = 0.002f;
    static int blurRadius = 5;

    ImGui::SeparatorText("Screenspace Controls");

    if (ImGui::TreeNode("SSAO Settings"))
    {
        ImGui::SliderFloat("Occlusion Radius", &occlusionRadius, 0.001f, 5.0f, "%.3f");
        ImGui::SliderFloat("Fade Start", &occlusionFadeStart, 0.0f, 10.0f, "%.3f");
        ImGui::SliderFloat("Fade End", &occlusionFadeEnd, 1.0f, 10.0f, "%.3f");
        ImGui::SliderFloat("Surface Epsilon", &surfaceEpsilon, 0.00001f, 0.5f, "%.5f");
        ImGui::SliderInt("Blur Radius", &blurRadius, 1, 5);
        ImGui::TreePop();
    }

    ssaoCB.OcclusionRadius = occlusionRadius;
    ssaoCB.OcclusionFadeStart = occlusionFadeStart;
    ssaoCB.OcclusionFadeEnd = occlusionFadeEnd;
    ssaoCB.SurfaceEpsilon = surfaceEpsilon;
    ssaoVerticalBlurCB.BlurRadius = blurRadius;
    ssaoHorizontalBlurCB.BlurRadius = blurRadius;
    ssaoHorizontalBlurCB = ssaoVerticalBlurCB;
    ssaoVerticalBlurCB.HorizontalBlur = 0;
    ssaoHorizontalBlurCB.HorizontalBlur = 1;

    ssgiCB.Proj = ssaoCB.Proj;
    ssgiCB.InvProj = ssaoCB.InvProj;
    ssgiCB.ProjTex = ssaoCB.ProjTex;

    for (int k = 0; k < 14; k++)
        ssgiCB.OffsetVectors[k] = ssaoCB.OffsetVectors[k];

    static float giSampleRadius = 0.02f;
    static float giFalloffScale = 0.012f;
    static int giBlurRadius = 5;

    if (ImGui::TreeNode("SSGI Settings"))
    {
        ImGui::SliderFloat("GI Sample Radius", &giSampleRadius, 0.00001f, 1.5f, "%.5f");
        ImGui::SliderFloat("GI Falloff Scale", &giFalloffScale, 0.00001f, 1.5f, "%.5f");
        ImGui::SliderInt("GI Blur Radius", &giBlurRadius, 1, 5);
        ImGui::TreePop();
    }

    ssgiCB.GiSampleRadius = giSampleRadius;
    ssgiCB.GiFalloffScale = giFalloffScale;

    ssgiVerticalBlurCB.Proj = ssaoVerticalBlurCB.Proj;
    ssgiVerticalBlurCB.BlurWeights[0] = ssaoVerticalBlurCB.BlurWeights[0];
    ssgiVerticalBlurCB.BlurWeights[1] = ssaoVerticalBlurCB.BlurWeights[1];
    ssgiVerticalBlurCB.BlurWeights[2] = ssaoVerticalBlurCB.BlurWeights[2];
    ssgiVerticalBlurCB.InvRenderTargetSize = ssaoVerticalBlurCB.InvRenderTargetSize;
    ssgiVerticalBlurCB.BlurRadius = giBlurRadius;
    ssgiVerticalBlurCB.HorizontalBlur = 0;
    ssgiHorizontalBlurCB = ssgiVerticalBlurCB;
    ssgiHorizontalBlurCB.HorizontalBlur = 1;

    SssBlurConstants sssBlurCB;
    sssBlurCB.Proj = ssaoVerticalBlurCB.Proj;
    sssBlurCB.BlurWeights[0] = ssaoVerticalBlurCB.BlurWeights[0];
    sssBlurCB.BlurWeights[1] = ssaoVerticalBlurCB.BlurWeights[1];
    sssBlurCB.BlurWeights[2] = ssaoVerticalBlurCB.BlurWeights[2];
    sssBlurCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / sss->SSSWidth(), 1.0f / sss->SSSHeight());
    sssBlurCB.BlurRadius = 4;
    sssBlurCB.HorizontalBlur = 0;

    fr->SsaoCB->CopyData(0, ssaoCB);
    fr->SsaoVerticalBlurCB->CopyData(0, ssaoVerticalBlurCB);
    fr->SsaoHorizontalBlurCB->CopyData(0, ssaoHorizontalBlurCB);
    fr->SsgiCB->CopyData(0, ssgiCB);
    fr->SsgiVerticalBlurCB->CopyData(0, ssgiVerticalBlurCB);
    fr->SsgiHorizontalBlurCB->CopyData(0, ssgiHorizontalBlurCB);
    fr->SssBlurCB->CopyData(0, sssBlurCB);
}
