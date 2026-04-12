#include "../EngineApp.h"

inline float DegreesToRadians(float degrees)
{
    return degrees * (DirectX::XM_PI / 180.0f);
}

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

        UINT w = mShadowResourceArray->Get<ShadowPassResource>(lightIndex)->Width();
        UINT h = mShadowResourceArray->Get<ShadowPassResource>(lightIndex)->Height();

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

    for (int i = 0; i < dynamicLights.SpotLights.size(); i++, lightIndex++)
    {
        DirectX::XMMATRIX view = XMLoadFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewMatrix);
        DirectX::XMMATRIX proj = XMLoadFloat4x4(&dynamicLights.LightTransforms[lightIndex].ProjectionMatrix);

        DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
        DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
        DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		UINT w = mShadowResourceArray->Get<ShadowPassResource>(lightIndex)->Width();
		UINT h = mShadowResourceArray->Get<ShadowPassResource>(lightIndex)->Height();

        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvView, XMMatrixTranspose(invView));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].Proj, XMMatrixTranspose(proj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvProj, XMMatrixTranspose(invProj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].ViewProj, XMMatrixTranspose(viewProj));
        XMStoreFloat4x4(&mShadowPassCBs[lightIndex].InvViewProj, XMMatrixTranspose(invViewProj));
        mShadowPassCBs[lightIndex].EyePosW = dynamicLights.SpotLights[i].Position;
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

        if (n < 0.1f) n = 0.1f;

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

    for (int i = 0; i < dynamicLights.SpotLights.size(); i++, lightIndex++)
    {
        DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&mMainPassCB.Lights[lightIndex].Direction);
        DirectX::XMVECTOR lightPos = XMLoadFloat3(&mMainPassCB.Lights[lightIndex].Position);
        
        DirectX::XMVECTOR lightUp = XMLoadFloat3(&up);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, DirectX::XMVectorAdd(lightPos,  DirectX::XMVector3Normalize(lightDir)), lightUp);

        dynamicLights.LightTransforms[lightIndex].NearZ = mMainPassCB.Lights[lightIndex].FalloffStart;
        dynamicLights.LightTransforms[lightIndex].FarZ = mMainPassCB.Lights[lightIndex].FalloffEnd;

        // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
        DirectX::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
            
        float fov = DegreesToRadians(mMainPassCB.Lights[lightIndex].OuterConeAngle * 2.0f);
        DirectX::XMMATRIX lightProj = DirectX::XMMatrixPerspectiveFovLH(fov, 1.0f, dynamicLights.LightTransforms[lightIndex].NearZ, dynamicLights.LightTransforms[lightIndex].FarZ);

        DirectX::XMMATRIX S = lightView * lightProj * T;
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewMatrix, lightView);
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ProjectionMatrix, lightProj);
        XMStoreFloat4x4(&dynamicLights.LightTransforms[lightIndex].ViewProjectionMatrix, S);
    }
}