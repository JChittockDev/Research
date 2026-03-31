#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Render/Resources/FrameResource.h"
#include "../Utilities/GameTimer.h"
#include "../ImGui/imgui.h"
#include <DirectXMath.h>

void UpdateSssCB(const GameTimer& gt, SceneState& state, FrameResource* fr)
{
    SssConstants sssCB;

    for (int i = 0; i < state.lights.GetNumLights(); i++)
    {
        sssCB.Lights[i] = state.mainPassCB.Lights[i];
    }

    // Recover original (non-transposed) projection matrix from stored transposed value
    DirectX::XMMATRIX P = XMMatrixTranspose(XMLoadFloat4x4(&state.mainPassCB.Proj));

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    sssCB.Proj = state.mainPassCB.Proj;
    sssCB.InvProj = state.mainPassCB.InvProj;
    sssCB.ViewProj = state.mainPassCB.ViewProj;
    sssCB.EyePosW = state.mainPassCB.EyePosW;
    XMStoreFloat4x4(&sssCB.ProjTex, XMMatrixTranspose(P * T));

    static float blend = 1.0f;
    static float thickness = 0.5f;
    static float scale = 0.5f;
    static DirectX::XMFLOAT3 transmissionColor(0.7f, 0.6f, 0.5f);
    static DirectX::XMFLOAT3 scatteringProfile(0.05f, 0.01f, 0.01f);
    static int sampleCount = 8;

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

    fr->SssCB->CopyData(0, sssCB);
}
