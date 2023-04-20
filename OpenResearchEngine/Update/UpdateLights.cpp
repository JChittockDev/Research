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
        DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSet(dynamicLights.SpotLights[i].Direction.x, 
                                                                                                              -dynamicLights.SpotLights[i].Direction.y, 
                                                                                                              dynamicLights.SpotLights[i].Direction.z, 0.f)), bounds);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationMatrix(lightView));
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(dynamicLights.LightTransforms[i].ReferencePosition, rotation, scale);
        std::string renderItemPath = "Models\\directionallight.obj_" + std::to_string(i);

        for (int b = 0; b < mLightRenderItemMap[renderItemPath].size(); b++)
        {
            if (mLightRenderItemMap[renderItemPath][b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mLightRenderItemMap[renderItemPath][b]->TexTransform)));
                objConstants.MaterialIndex = mLightRenderItemMap[renderItemPath][b]->Mat->MatCBIndex;
                currObjectCB->CopyData(mLightRenderItemMap[renderItemPath][b]->ObjCBIndex, objConstants);

                // Next FrameResource need to be updated too.
                mLightRenderItemMap[renderItemPath][b]->NumFramesDirty--;
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
        std::string renderItemPath = "Models\\spotlight.obj_" + std::to_string(i);

        for (int b = 0; b < mLightRenderItemMap[renderItemPath].size(); b++)
        {
            if (mLightRenderItemMap[renderItemPath][b]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&transformMatrix);
                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(transform));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mLightRenderItemMap[renderItemPath][b]->TexTransform)));
                objConstants.MaterialIndex = mLightRenderItemMap[renderItemPath][b]->Mat->MatCBIndex;
                currObjectCB->CopyData(mLightRenderItemMap[renderItemPath][b]->ObjCBIndex, objConstants);

                // Next FrameResource need to be updated too.
                mLightRenderItemMap[renderItemPath][b]->NumFramesDirty--;
            }
        }
    }
}