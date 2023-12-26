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
    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    UpdateLightTransforms(dynamicLights.LightTransforms, mMainPassCB.LightTransforms);

    float bounds = -2.0f * mSceneBounds.Radius;
    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
    {
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
        std::string renderItemPath = "defaultobject_" + std::to_string(i);

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
    }

    for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
    {
        DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
        DirectX::XMVECTOR lightDir = DirectX::XMVector3Normalize(DirectX::XMVectorSet(-dynamicLights.SpotLights[i].Direction.x, dynamicLights.SpotLights[i].Direction.y, dynamicLights.SpotLights[i].Direction.z, 0.f));
        DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&dynamicLights.SpotLights[i].Position);
        DirectX::XMVECTOR lightUp = XMLoadFloat3(&up);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, DirectX::XMVectorAdd(lightPos, DirectX::XMVector3Normalize(lightDir)), lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(lightView));
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(dynamicLights.SpotLights[i].Position, rotation, scale);
        std::string renderItemPath = "defaultobject_" + std::to_string(i);

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
    }
}