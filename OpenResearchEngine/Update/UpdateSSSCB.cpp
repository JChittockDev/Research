#include "../EngineApp.h"

void EngineApp::UpdateSssCB(const GameTimer& gt)
{
    SssConstants sssCB;

    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        sssCB.Lights[i] = mMainPassCB.Lights[i];
    }

    DirectX::XMMATRIX P = mCamera.GetProj();

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    sssCB.Proj = mMainPassCB.Proj;
    sssCB.InvProj = mMainPassCB.InvProj;
    sssCB.ViewProj = mMainPassCB.ViewProj;
    sssCB.EyePosW = mMainPassCB.EyePosW;
    XMStoreFloat4x4(&sssCB.ProjTex, XMMatrixTranspose(P * T));

    // Default SSAO settings if ImGui is not modifying them
    static float blend = 1.0f;
    static float thickness = 0.5f;
    static float scale = 3.5f;
    static DirectX::XMFLOAT3 transmissionColor(0.8, 0.6, 0.5);
    static DirectX::XMFLOAT3 scatteringProfile(0.003, 0.005, 0.007);
    static int sampleCount = 64;

    if (ImGui::TreeNode("SSS Settings"))
    {
        ImGui::DragFloat3("Transmission Color", &transmissionColor.x);
        ImGui::DragFloat3("Scattering Profile", &scatteringProfile.x);
        ImGui::SliderFloat("Blend", &blend, 0.001f, 1.0f, "%.3f");
        ImGui::SliderFloat("Thickness", &thickness, 0.001f, 10.0f, "%.3f");
        ImGui::SliderFloat("Scale", &scale, 0.001f, 10.0f, "%.3f");
        ImGui::SliderInt("Sample Count", &sampleCount, 1, 64);
        ImGui::TreePop();
    }

    sssCB.TransmissionColor = transmissionColor;
    sssCB.ScatteringProfile = scatteringProfile;
    sssCB.Blend = blend;
    sssCB.Thickness = thickness;
    sssCB.SampleCount = sampleCount;
    sssCB.Scale = scale;

    auto currSssCB = mCurrFrameResource->SssCB.get();
    currSssCB->CopyData(0, sssCB);
}