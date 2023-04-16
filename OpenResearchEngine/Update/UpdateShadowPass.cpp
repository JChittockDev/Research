#include "../EngineApp.h"

void EngineApp::UpdateShadowPassCB(const GameTimer& gt)
{
    auto currPassCB = mCurrFrameResource->PassCB.get();

    int lightIndex = 0;
    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++, lightIndex++)
    {

        DirectX::XMMATRIX view = XMLoadFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewMatrix);
        DirectX::XMMATRIX proj = XMLoadFloat4x4(&dynamicLights.LightTransforms[lightIndex].ProjectionMatrix);

        DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
        DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
        DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

        UINT w = mShadowMaps[i]->Width();
        UINT h = mShadowMaps[i]->Height();

        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvView, XMMatrixTranspose(invView));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].Proj, XMMatrixTranspose(proj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvProj, XMMatrixTranspose(invProj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].ViewProj, XMMatrixTranspose(viewProj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvViewProj, XMMatrixTranspose(invViewProj));
        mShadowPassCBs[lightIndex].EyePosW = dynamicLights.DirectionalLights[i].Position;
        mShadowPassCBs[lightIndex].RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
        mShadowPassCBs[lightIndex].InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
        mShadowPassCBs[lightIndex].NearZ = dynamicLights.LightTransforms[lightIndex].NearZ;
        mShadowPassCBs[lightIndex].FarZ = dynamicLights.LightTransforms[lightIndex].FarZ;

        currPassCB->CopyData(lightIndex + 1, mShadowPassCBs[lightIndex]);
    }
}

void EngineApp::UpdateShadowTransform(const GameTimer& gt)
{
    // Only the first "main" light casts a shadow.
    float bounds = -2.0f * mSceneBounds.Radius;

    int lightIndex = 0;
    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++, lightIndex++)
    {
        DirectX::XMFLOAT3 fauxPosition = DirectX::XMFLOAT3(bounds * mMainPassCB.Lights[lightIndex].Direction.x, bounds * mMainPassCB.Lights[lightIndex].Direction.y, bounds * mMainPassCB.Lights[lightIndex].Direction.z);
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&mMainPassCB.Lights[lightIndex].Direction);
        DirectX::XMVECTOR lightPos = XMLoadFloat3(&fauxPosition);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        XMStoreFloat3(&dynamicLights.DirectionalLights[i].Position, lightPos);

        // Transform bounding sphere to light space.
        DirectX::XMFLOAT3 sphereCenterLS;
        XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

        // Ortho frustum in light space encloses scene.
        float l = sphereCenterLS.x - mSceneBounds.Radius;
        float b = sphereCenterLS.y - mSceneBounds.Radius;
        float n = sphereCenterLS.z - mSceneBounds.Radius;
        float r = sphereCenterLS.x + mSceneBounds.Radius;
        float t = sphereCenterLS.y + mSceneBounds.Radius;
        float f = sphereCenterLS.z + mSceneBounds.Radius;

        dynamicLights.LightTransforms[lightIndex].NearZ = n;
        dynamicLights.LightTransforms[lightIndex].FarZ = f;
        
        DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

        // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
        DirectX::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

        DirectX::XMMATRIX S = lightView * lightProj * T;
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewMatrix, lightView);
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ProjectionMatrix, lightProj);
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewProjectionMatrix, S);
    }
}