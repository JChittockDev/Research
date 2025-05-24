#include "../EngineApp.h"

void EngineApp::UpdateSsaoCB(const GameTimer& gt)
{
    SsaoConstants ssaoCB;

    DirectX::XMMATRIX P = mCamera.GetProj();

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    ssaoCB.Proj = mMainPassCB.Proj;
    ssaoCB.InvProj = mMainPassCB.InvProj;
    XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

    mSsao->GetOffsetVectors(ssaoCB.OffsetVectors);

    // Default SSAO settings if ImGui is not modifying them
    static float occlusionRadius = 0.5f;
    static float occlusionFadeStart = 0.2f;
    static float occlusionFadeEnd = 2.0f;
    static float surfaceEpsilon = 0.05f;

    ImGui::SeparatorText("Screenspace Controls");

    if (ImGui::TreeNode("SSAO Settings"))
    {
        ImGui::SliderFloat("Occlusion Radius", &occlusionRadius, 0.1f, 2.0f, "%.2f");
        ImGui::SliderFloat("Fade Start", &occlusionFadeStart, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Fade End", &occlusionFadeEnd, 1.0f, 10.0f, "%.2f");
        ImGui::SliderFloat("Surface Epsilon", &surfaceEpsilon, 0.001f, 0.1f, "%.3f");
        ImGui::TreePop();
    }

    ssaoCB.OcclusionRadius = occlusionRadius;
    ssaoCB.OcclusionFadeStart = occlusionFadeStart;
    ssaoCB.OcclusionFadeEnd = occlusionFadeEnd;
    ssaoCB.SurfaceEpsilon = surfaceEpsilon;

    auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
    currSsaoCB->CopyData(0, ssaoCB);
}