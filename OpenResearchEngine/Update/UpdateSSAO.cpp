#include "../EngineApp.h"

void EngineApp::UpdateSsaoCB(const GameTimer& gt)
{
    SsaoConstants ssaoCB;
    SsaoBlurConstants ssaoVerticalBlurCB;
    SsaoBlurConstants ssaoHorizontalBlurCB;

    DirectX::XMMATRIX P = mCamera.GetProj();

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    ssaoCB.Proj = mMainPassCB.Proj;
    ssaoVerticalBlurCB.Proj = mMainPassCB.Proj;
    ssaoCB.InvProj = mMainPassCB.InvProj;
    XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

    mSsao->GetOffsetVectors(ssaoCB.OffsetVectors);

    auto blurWeights = mSsao->CalcGaussWeights(2.5f);
    ssaoVerticalBlurCB.BlurWeights[0] = DirectX::XMFLOAT4(&blurWeights[0]);
    ssaoVerticalBlurCB.BlurWeights[1] = DirectX::XMFLOAT4(&blurWeights[4]);
    ssaoVerticalBlurCB.BlurWeights[2] = DirectX::XMFLOAT4(&blurWeights[8]);
    ssaoVerticalBlurCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mSsao->SsaoWidth(), 1.0f / mSsao->SsaoHeight());
    ssaoVerticalBlurCB.BlurRadius = 5;

    // Default SSAO settings if ImGui is not modifying them
    static float occlusionRadius = 0.05f;
    static float occlusionFadeStart = 0.2f;
    static float occlusionFadeEnd = 2.0f;
    static float surfaceEpsilon = 0.065f;

    static int blurRadius = 5;

    ImGui::SeparatorText("Screenspace Controls");

    if (ImGui::TreeNode("SSAO Settings"))
    {
        ImGui::SliderFloat("Occlusion Radius", &occlusionRadius, 0.001f, 5.0f, "%.3f");
        ImGui::SliderFloat("Fade Start", &occlusionFadeStart, 0.0f, 10.0f, "%.3f");
        ImGui::SliderFloat("Fade End", &occlusionFadeEnd, 1.0f, 10.0f, "%.3f");
        ImGui::SliderFloat("Surface Epsilon", &surfaceEpsilon, 0.001f, 0.5f, "%.3f");
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

    auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
    auto currSsaoVerticalBlurCB = mCurrFrameResource->SsaoVerticalBlurCB.get();
    auto currSsaoHorizontalBlurCB = mCurrFrameResource->SsaoHorizontalBlurCB.get();

    currSsaoCB->CopyData(0, ssaoCB);
    currSsaoVerticalBlurCB->CopyData(0, ssaoVerticalBlurCB);
    currSsaoHorizontalBlurCB->CopyData(0, ssaoHorizontalBlurCB);
}