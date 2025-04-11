#include "../EngineApp.h"

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

void EngineApp::UpdateLights(const GameTimer& gt)
{
    ImGui::SeparatorText("Lights");

    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    UpdateLightTransforms(dynamicLights.LightTransforms, mMainPassCB.LightTransforms);

    int count = 0;

    float bounds = -2.0f * mSceneBounds.Radius;
    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
    {
        std::string renderItemPath = "defaultobject_" + std::to_string(i);
        std::string lightLabel = "Directional Light " + std::to_string(i);
        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&dynamicLights.DirectionalLights[i].Strength));
            ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&dynamicLights.DirectionalLights[i].Direction));

            for (int b = 0; b < mDirectionalLightRenderItemMap.at(renderItemPath).size(); b++)
            {
                mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;
            }

            mMainPassCB.Lights[count].Strength = dynamicLights.DirectionalLights[i].Strength;
            mMainPassCB.Lights[count].Direction = dynamicLights.DirectionalLights[i].Direction;

            ImGui::TreePop();
        }

        DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSet(dynamicLights.DirectionalLights[i].Direction.x,
                                                                                                              -dynamicLights.DirectionalLights[i].Direction.y,
                                                                                                              dynamicLights.DirectionalLights[i].Direction.z, 0.f)), bounds);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(lightView));
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(dynamicLights.LightTransforms[i].ReferencePosition, rotation, scale);

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

                // Next FrameResource need to be updated too.
                mDirectionalLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }
        count++;
    }

    for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
    {
        std::string lightLabel = "Spot Light " + std::to_string(i);
        std::string renderItemPath = "defaultobject_" + std::to_string(i);
        if (ImGui::TreeNode(lightLabel.c_str()))
        {
            ImGui::DragFloat3("Strength", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Strength));
            ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Direction));
            ImGui::DragFloat3("Position", reinterpret_cast<float*>(&dynamicLights.SpotLights[i].Position));

            for (int b = 0; b < mSpotLightRenderItemMap.at(renderItemPath).size(); b++)
            {
                mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty = gNumFrameResources;
            }

            mMainPassCB.Lights[count].Strength = dynamicLights.SpotLights[i].Strength;
            mMainPassCB.Lights[count].Direction = dynamicLights.SpotLights[i].Direction;
            mMainPassCB.Lights[count].Position = dynamicLights.SpotLights[i].Position;

            ImGui::TreePop();
        }


        DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
        DirectX::XMVECTOR lightDir = DirectX::XMVector3Normalize(DirectX::XMVectorSet(-dynamicLights.SpotLights[i].Direction.x, dynamicLights.SpotLights[i].Direction.y, dynamicLights.SpotLights[i].Direction.z, 0.f));
        DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&dynamicLights.SpotLights[i].Position);
        DirectX::XMVECTOR lightUp = XMLoadFloat3(&up);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, DirectX::XMVectorAdd(lightPos, DirectX::XMVector3Normalize(lightDir)), lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(lightView));
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

                // Next FrameResource need to be updated too.
                mSpotLightRenderItemMap.at(renderItemPath)[b]->NumFramesDirty--;
            }
        }
        count++;
    }

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}