#include "../EngineApp.h"
#include <DirectXMath.h>

void EngineApp::UpdateLightTransforms(const std::vector<LightTransform>& lights, DirectX::XMFLOAT4X4* LightTransforms)
{
    for (int i = 0; i < lights.size(); i++)
    {
        DirectX::XMMATRIX transposeMatrix = XMMatrixTranspose(XMLoadFloat4x4(&lights[i].ViewProjectionMatrix));
        DirectX::XMFLOAT4X4 out;
        XMStoreFloat4x4(&out, transposeMatrix);
        LightTransforms[i] = out;
    }
}

// Helper function to convert degrees to radians
inline float DegreesToRadians(float degrees)
{
    return degrees * (DirectX::XM_PI / 180.0f);
}

// Helper function to create rotation quaternion from Euler angles in degrees
DirectX::XMFLOAT4 CreateRotationQuaternionFromDegrees(const DirectX::XMFLOAT3& eulerDegrees)
{
    // Convert degrees to radians
    float pitch = DegreesToRadians(eulerDegrees.x);
    float yaw = DegreesToRadians(eulerDegrees.y);
    float roll = DegreesToRadians(eulerDegrees.z);

    // Create quaternion from Euler angles (YXZ order - yaw, pitch, roll)
    DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

    DirectX::XMFLOAT4 result;
    DirectX::XMStoreFloat4(&result, quat);
    return result;
}

// Helper function to create direction vector by rotating default direction (0,0,1) by Euler angles in degrees
DirectX::XMFLOAT3 CreateDirectionFromDegrees(const DirectX::XMFLOAT3& eulerDegrees)
{
    // Convert degrees to radians
    float pitch = DegreesToRadians(eulerDegrees.x);
    float yaw = DegreesToRadians(eulerDegrees.y);
    float roll = DegreesToRadians(eulerDegrees.z);

    // Create rotation matrix from Euler angles (YXZ order - yaw, pitch, roll)
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Default direction vector (0, 0, 1)
    DirectX::XMVECTOR defaultDirection = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    // Apply rotation to the default direction
    DirectX::XMVECTOR rotatedDirection = DirectX::XMVector3TransformNormal(defaultDirection, rotationMatrix);

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, rotatedDirection);
    return result;
}

void EngineApp::UpdateLights(const GameTimer& gt)
{
    ImGui::SeparatorText("Lights");

    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    UpdateLightTransforms(dynamicLights.LightTransforms, mMainPassCB.LightTransforms);

    int count = 0;

    // Set the constant buffer data for the lights in the main pass CB
    float bounds = -2.0f * mSceneBounds.Radius;

    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
    {
        std::string renderItemPath = "defaultobject_" + std::to_string(i);
        std::string lightLabel = "Directional Light " + std::to_string(i);

        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&dynamicLights.DirectionalLights[i].Strength));

            // Use Euler angles in degrees for ImGui input
            ImGui::DragFloat3("Direction (Degrees)", reinterpret_cast<float*>(&dynamicLights.DirectionalLights[i].Direction), 1.0f, -360.0f, 360.0f);

            ImGui::TreePop();
        }

        // Update the render items for the lights - position the light representation geometry
        // Get the actual direction vector by rotating the default direction
        DirectX::XMFLOAT3 actualDirection = CreateDirectionFromDegrees(dynamicLights.DirectionalLights[i].Direction);
        DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&actualDirection);
        DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(lightDirection), bounds);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        // Create look-at matrix for the light representation
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);

        // Extract rotation quaternion from the view matrix
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixInverse(nullptr, lightView)));

        DirectX::XMFLOAT3 lightPosition;
        DirectX::XMStoreFloat3(&lightPosition, lightPos);
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

        // Use the reference position from LightTransforms or calculated position
        DirectX::XMFLOAT3 finalPosition = dynamicLights.LightTransforms[i].ReferencePosition;
        if (finalPosition.x == 0 && finalPosition.y == 0 && finalPosition.z == 0)
        {
            finalPosition = lightPosition; // Use calculated position if reference is zero
        }

        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(finalPosition, rotation, scale);

        for (int b = 0; b < mDirectionalLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            if (mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mDirectionalLightRenderItemMap.at(renderItemPath)[b]->TexTransform)));
                objConstants.MaterialIndex = mDirectionalLightRenderItemMap.at(renderItemPath)[b]->Mat->MaterialIndex;
                currObjectCB->CopyData(mDirectionalLightRenderItemMap.at(renderItemPath)[b]->ObjCBIndex, objConstants);

                mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }

        for (int b = 0; b < mDirectionalLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;
        }

        mMainPassCB.Lights[count].Strength = dynamicLights.DirectionalLights[i].Strength;
        mMainPassCB.Lights[count].Direction = CreateDirectionFromDegrees(dynamicLights.DirectionalLights[i].Direction);
        mMainPassCB.Lights[count].Type = dynamicLights.DirectionalLights[i].Type;
        count++;
    }

    // Spot lights with degree-based input
    for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
    {
        std::string lightLabel = "Spot Light " + std::to_string(i);
        std::string renderItemPath = "defaultobject_" + std::to_string(i);

        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Strength));
            ImGui::DragFloat3("Position", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Position));

            // Use Euler angles in degrees for ImGui input
            ImGui::DragFloat3("Direction (Degrees)", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Direction), 1.0f, -360.0f, 360.0f);

            // Spot light specific parameters
            ImGui::DragFloat("Inner Cone Angle", &dynamicLights.SpotLights[i].InnerConeAngle, 1.0f, 0.0f, 90.0f, "%.1f");
            ImGui::DragFloat("Outer Cone Angle", &dynamicLights.SpotLights[i].OuterConeAngle, 1.0f, 0.0f, 90.0f, "%.1f");
            ImGui::DragFloat("Falloff Start", &dynamicLights.SpotLights[i].FalloffStart, 0.1f, 0.0f, 100.0f, "%.2f");
            ImGui::DragFloat("Falloff End", &dynamicLights.SpotLights[i].FalloffEnd, 0.1f, 0.0f, 100.0f, "%.2f");

            ImGui::TreePop();
        }

        // Create transform for spot light representation geometry
        // Get the actual direction vector by rotating the default direction
        DirectX::XMFLOAT3 actualDirection = CreateDirectionFromDegrees(dynamicLights.SpotLights[i].Direction);
        DirectX::XMVECTOR lightDirection = DirectX::XMLoadFloat3(&actualDirection);
        DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&dynamicLights.SpotLights[i].Position);
        DirectX::XMVECTOR targetPos = DirectX::XMVectorAdd(lightPos, DirectX::XMVector3Normalize(lightDirection));
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        // Create look-at matrix
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);

        // Extract rotation quaternion from the view matrix
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixInverse(nullptr, lightView)));

        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(dynamicLights.SpotLights[i].Position, rotation, scale);

        for (int b = 0; b < mSpotLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            if (mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mSpotLightRenderItemMap.at(renderItemPath)[b]->TexTransform)));
                objConstants.MaterialIndex = mSpotLightRenderItemMap.at(renderItemPath)[b]->Mat->MaterialIndex;
                currObjectCB->CopyData(mSpotLightRenderItemMap.at(renderItemPath)[b]->ObjCBIndex, objConstants);

                mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }

        for (int b = 0; b < mSpotLightRenderItemMap.at(renderItemPath).size(); b++)
        {
            mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;
        }

        // Ensure inner cone is always smaller than outer cone
        if (dynamicLights.SpotLights[i].InnerConeAngle > dynamicLights.SpotLights[i].OuterConeAngle)
        {
            dynamicLights.SpotLights[i].InnerConeAngle = dynamicLights.SpotLights[i].OuterConeAngle;
        }

        // Ensure falloff start is less than falloff end
        if (dynamicLights.SpotLights[i].FalloffStart > dynamicLights.SpotLights[i].FalloffEnd)
        {
            dynamicLights.SpotLights[i].FalloffStart = dynamicLights.SpotLights[i].FalloffEnd;
        }

        mMainPassCB.Lights[count].Strength = dynamicLights.SpotLights[i].Strength;
        mMainPassCB.Lights[count].Direction = CreateDirectionFromDegrees(dynamicLights.SpotLights[i].Direction);
        mMainPassCB.Lights[count].Position = dynamicLights.SpotLights[i].Position;
        mMainPassCB.Lights[count].FalloffStart = dynamicLights.SpotLights[i].FalloffStart;
        mMainPassCB.Lights[count].FalloffEnd = dynamicLights.SpotLights[i].FalloffEnd;
        mMainPassCB.Lights[count].InnerConeAngle = dynamicLights.SpotLights[i].InnerConeAngle;
        mMainPassCB.Lights[count].OuterConeAngle = dynamicLights.SpotLights[i].OuterConeAngle;
        mMainPassCB.Lights[count].Type = dynamicLights.SpotLights[i].Type;

        count++;
    }

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}