#include "../EngineApp.h"

void EngineApp::UpdateScreenSpaceCB(const GameTimer& gt)
{
    SsaoConstants ssaoCB;
    SsaoBlurConstants ssaoVerticalBlurCB;
    SsaoBlurConstants ssaoHorizontalBlurCB;

    SsgiConstants ssgiCB;
    SsgiBlurConstants ssgiVerticalBlurCB;
    SsgiBlurConstants ssgiHorizontalBlurCB;

    DirectX::XMMATRIX P = mCamera.GetProj();

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    ssaoCB.Proj = mMainPassCB.Proj;
    ssaoCB.InvProj = mMainPassCB.InvProj;
    ssaoVerticalBlurCB.Proj = mMainPassCB.Proj;
    XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

    mSsao->GetOffsetVectors(ssaoCB.OffsetVectors);

    auto blurWeights = mSsao->CalcGaussWeights(2.5f);
    ssaoVerticalBlurCB.BlurWeights[0] = DirectX::XMFLOAT4(&blurWeights[0]);
    ssaoVerticalBlurCB.BlurWeights[1] = DirectX::XMFLOAT4(&blurWeights[4]);
    ssaoVerticalBlurCB.BlurWeights[2] = DirectX::XMFLOAT4(&blurWeights[8]);
    ssaoVerticalBlurCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mSsao->GetRenderWidth(), 1.0f / mSsao->GetRenderHeight());
    ssaoVerticalBlurCB.BlurRadius = 5;

    // Default SSAO settings if ImGui is not modifying them
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

    ssgiCB.OffsetVectors[0] = ssaoCB.OffsetVectors[0];
    ssgiCB.OffsetVectors[1] = ssaoCB.OffsetVectors[1];
    ssgiCB.OffsetVectors[2] = ssaoCB.OffsetVectors[2];
    ssgiCB.OffsetVectors[3] = ssaoCB.OffsetVectors[3];
    ssgiCB.OffsetVectors[4] = ssaoCB.OffsetVectors[4];
    ssgiCB.OffsetVectors[5] = ssaoCB.OffsetVectors[5];
    ssgiCB.OffsetVectors[6] = ssaoCB.OffsetVectors[6];
    ssgiCB.OffsetVectors[7] = ssaoCB.OffsetVectors[7];
    ssgiCB.OffsetVectors[8] = ssaoCB.OffsetVectors[8];
    ssgiCB.OffsetVectors[9] = ssaoCB.OffsetVectors[9];
    ssgiCB.OffsetVectors[10] = ssaoCB.OffsetVectors[10];
    ssgiCB.OffsetVectors[11] = ssaoCB.OffsetVectors[11];
    ssgiCB.OffsetVectors[12] = ssaoCB.OffsetVectors[12];
    ssgiCB.OffsetVectors[13] = ssaoCB.OffsetVectors[13];

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
    sssBlurCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mSss->SSSWidth(), 1.0f / mSss->SSSHeight());
    sssBlurCB.BlurRadius = 4;
    sssBlurCB.HorizontalBlur = 0;

    auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
    auto currSsaoVerticalBlurCB = mCurrFrameResource->SsaoVerticalBlurCB.get();
    auto currSsaoHorizontalBlurCB = mCurrFrameResource->SsaoHorizontalBlurCB.get();
    auto currSsgiCB = mCurrFrameResource->SsgiCB.get();
    auto currSsgiVerticalBlurCB = mCurrFrameResource->SsgiVerticalBlurCB.get();
    auto currSsgiHorizontalBlurCB = mCurrFrameResource->SsgiHorizontalBlurCB.get();
    auto currSssBlurCB = mCurrFrameResource->SssBlurCB.get();

    currSsaoCB->CopyData(0, ssaoCB);
    currSsaoVerticalBlurCB->CopyData(0, ssaoVerticalBlurCB);
    currSsaoHorizontalBlurCB->CopyData(0, ssaoHorizontalBlurCB);
    currSsgiCB->CopyData(0, ssgiCB);
    currSsgiVerticalBlurCB->CopyData(0, ssgiVerticalBlurCB);
    currSsgiHorizontalBlurCB->CopyData(0, ssgiHorizontalBlurCB);
    currSssBlurCB->CopyData(0, sssBlurCB);
}