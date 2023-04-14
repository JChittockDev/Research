#include "../EngineApp.h"

void EngineApp::UpdateShadowPassCB2(const GameTimer& gt)
{
    DirectX::XMMATRIX view = XMLoadFloat4x4(&mLightView2);
    DirectX::XMMATRIX proj = XMLoadFloat4x4(&mLightProj2);

    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    UINT w = mShadowMap2->Width();
    UINT h = mShadowMap2->Height();

    XMStoreFloat4x4(&mShadowPassCB2.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mShadowPassCB2.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mShadowPassCB2.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mShadowPassCB2.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mShadowPassCB2.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mShadowPassCB2.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB2.EyePosW = mLightPosW2;
    mShadowPassCB2.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB2.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB2.NearZ = mLightNearZ2;
    mShadowPassCB2.FarZ = mLightFarZ2;

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(2, mShadowPassCB2);
}

void EngineApp::UpdateShadowTransform2(const GameTimer& gt)
{
    // Only the first "main" light casts a shadow.
    float bounds = -2.0f * mSceneBounds.Radius;
    DirectX::XMFLOAT3 fauxPosition = DirectX::XMFLOAT3(bounds * mMainPassCB.Lights[1].Direction.x, bounds * mMainPassCB.Lights[1].Direction.y, bounds * mMainPassCB.Lights[1].Direction.z);
    DirectX::XMVECTOR lightDir = XMLoadFloat3(&mMainPassCB.Lights[1].Direction);
    DirectX::XMVECTOR lightPos = XMLoadFloat3(&fauxPosition);
    DirectX::XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
    XMStoreFloat3(&mLightPosW2, lightPos);

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

    mLightNearZ2 = n;
    mLightFarZ2 = f;
    DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    DirectX::XMMATRIX S = lightView * lightProj * T;
    XMStoreFloat4x4(&mLightView2, lightView);
    XMStoreFloat4x4(&mLightProj2, lightProj);
    XMStoreFloat4x4(&mShadowTransform2, S);
}