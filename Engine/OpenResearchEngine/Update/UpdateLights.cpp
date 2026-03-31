#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Common/Math.h"
#include "../Render/Resources/FrameResource.h"
#include "../Assets/AssetManager.h"
#include "../Utilities/GameTimer.h"
#include <imgui.h>
#include <DirectXMath.h>

static void UpdateLightTransforms(const std::vector<LightTransform>& lights, DirectX::XMFLOAT4X4* LightTransforms)
{
    for (int i = 0; i < (int)lights.size(); i++)
    {
        DirectX::XMMATRIX transposeMatrix = XMMatrixTranspose(XMLoadFloat4x4(&lights[i].ViewProjectionMatrix));
        DirectX::XMFLOAT4X4 out;
        XMStoreFloat4x4(&out, transposeMatrix);
        LightTransforms[i] = out;
    }
}

inline float DegreesToRadiansLocal(float degrees)
{
    return degrees * (DirectX::XM_PI / 180.0f);
}

static DirectX::XMFLOAT4 CreateRotationQuaternionFromDegrees(const DirectX::XMFLOAT3& eulerDegrees)
{
    float pitch = DegreesToRadiansLocal(eulerDegrees.x);
    float yaw   = DegreesToRadiansLocal(eulerDegrees.y);
    float roll  = DegreesToRadiansLocal(eulerDegrees.z);
    DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
    DirectX::XMFLOAT4 result;
    DirectX::XMStoreFloat4(&result, quat);
    return result;
}

static DirectX::XMFLOAT3 CreateDirectionFromDegrees(const DirectX::XMFLOAT3& eulerDegrees)
{
    float pitch = DegreesToRadiansLocal(eulerDegrees.x);
    float yaw   = DegreesToRadiansLocal(eulerDegrees.y);
    float roll  = DegreesToRadiansLocal(eulerDegrees.z);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    DirectX::XMVECTOR defaultDirection = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR rotatedDirection = DirectX::XMVector3TransformNormal(defaultDirection, rotationMatrix);
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, rotatedDirection);
    return result;
}

void UpdateLights(const GameTimer& gt, SceneState& state, AssetManager& assets, FrameResource* fr)
{
    ImGui::SeparatorText("Lights");

    auto currObjectCB = fr->ObjectCB.get();
    UpdateLightTransforms(state.lights.LightTransforms, state.mainPassCB.LightTransforms);

    int count = 0;

    float bounds = -2.0f * state.sceneBounds.Radius;

    for (int i = 0; i < (int)state.lights.DirectionalLights.size(); i++)
    {
        std::string renderItemPath = "defaultobject_" + std::to_string(i);
        std::string lightLabel = "Directional Light " + std::to_string(i);

        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&state.lights.DirectionalLights[i].Strength));
            ImGui::DragFloat3("Direction (Degrees)", reinterpret_cast<float*>(&state.lights.DirectionalLights[i].Direction), 1.0f, -360.0f, 360.0f);
            ImGui::TreePop();
        }

        DirectX::XMFLOAT3 actualDirection = CreateDirectionFromDegrees(state.lights.DirectionalLights[i].Direction);
        DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&actualDirection);
        DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(lightDirection), bounds);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&state.sceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixInverse(nullptr, lightView)));

        DirectX::XMFLOAT3 lightPosition;
        DirectX::XMStoreFloat3(&lightPosition, lightPos);
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

        DirectX::XMFLOAT3 finalPosition = state.lights.LightTransforms[i].ReferencePosition;
        if (finalPosition.x == 0 && finalPosition.y == 0 && finalPosition.z == 0)
            finalPosition = lightPosition;

        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(finalPosition, rotation, scale);

        for (int b = 0; b < (int)assets.mDirectionalLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            if (assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->TexTransform)));
                objConstants.MaterialIndex = assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->Mat->MaterialIndex;
                currObjectCB->CopyData(assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->ObjCBIndex, objConstants);
                assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }

        for (int b = 0; b < (int)assets.mDirectionalLightRenderItemMap.at(renderItemPath).size(); b++)
            assets.mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;

        state.mainPassCB.Lights[count].Strength = state.lights.DirectionalLights[i].Strength;
        state.mainPassCB.Lights[count].Direction = CreateDirectionFromDegrees(state.lights.DirectionalLights[i].Direction);
        state.mainPassCB.Lights[count].Type = state.lights.DirectionalLights[i].Type;
        count++;
    }

    for (int i = 0; i < (int)state.lights.SpotLights.size(); i++)
    {
        std::string lightLabel = "Spot Light " + std::to_string(i);
        std::string renderItemPath = "defaultobject_" + std::to_string(i);

        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&state.lights.SpotLights[i].Strength));
            ImGui::DragFloat3("Position", reinterpret_cast<float*>(&state.lights.SpotLights[i].Position));
            ImGui::DragFloat3("Direction (Degrees)", reinterpret_cast<float*>(&state.lights.SpotLights[i].Direction), 1.0f, -360.0f, 360.0f);
            ImGui::DragFloat("Inner Cone Angle", &state.lights.SpotLights[i].InnerConeAngle, 1.0f, 0.0f, 90.0f, "%.1f");
            ImGui::DragFloat("Outer Cone Angle", &state.lights.SpotLights[i].OuterConeAngle, 1.0f, 0.0f, 90.0f, "%.1f");
            ImGui::DragFloat("Falloff Start", &state.lights.SpotLights[i].FalloffStart, 0.1f, 0.0f, 100.0f, "%.2f");
            ImGui::DragFloat("Falloff End", &state.lights.SpotLights[i].FalloffEnd, 0.1f, 0.0f, 100.0f, "%.2f");
            ImGui::TreePop();
        }

        DirectX::XMFLOAT3 actualDirection = CreateDirectionFromDegrees(state.lights.SpotLights[i].Direction);
        DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&actualDirection);
        DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&state.lights.SpotLights[i].Position);
        DirectX::XMVECTOR targetPos = DirectX::XMVectorAdd(lightPos, DirectX::XMVector3Normalize(lightDirection));
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixInverse(nullptr, lightView)));

        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(state.lights.SpotLights[i].Position, rotation, scale);

        for (int b = 0; b < (int)assets.mSpotLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            if (assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->TexTransform)));
                objConstants.MaterialIndex = assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->Mat->MaterialIndex;
                currObjectCB->CopyData(assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->ObjCBIndex, objConstants);
                assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }

        for (int b = 0; b < (int)assets.mSpotLightRenderItemMap.at(renderItemPath).size(); b++)
            assets.mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;

        if (state.lights.SpotLights[i].InnerConeAngle > state.lights.SpotLights[i].OuterConeAngle)
            state.lights.SpotLights[i].InnerConeAngle = state.lights.SpotLights[i].OuterConeAngle;

        if (state.lights.SpotLights[i].FalloffStart > state.lights.SpotLights[i].FalloffEnd)
            state.lights.SpotLights[i].FalloffStart = state.lights.SpotLights[i].FalloffEnd;

        state.mainPassCB.Lights[count].Strength = state.lights.SpotLights[i].Strength;
        state.mainPassCB.Lights[count].Direction = CreateDirectionFromDegrees(state.lights.SpotLights[i].Direction);
        state.mainPassCB.Lights[count].Position = state.lights.SpotLights[i].Position;
        state.mainPassCB.Lights[count].FalloffStart = state.lights.SpotLights[i].FalloffStart;
        state.mainPassCB.Lights[count].FalloffEnd = state.lights.SpotLights[i].FalloffEnd;
        state.mainPassCB.Lights[count].InnerConeAngle = state.lights.SpotLights[i].InnerConeAngle;
        state.mainPassCB.Lights[count].OuterConeAngle = state.lights.SpotLights[i].OuterConeAngle;
        state.mainPassCB.Lights[count].Type = state.lights.SpotLights[i].Type;

        count++;
    }

    fr->PassCB->CopyData(0, state.mainPassCB);
}
