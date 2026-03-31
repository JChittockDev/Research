#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Render/Resources/FrameResource.h"
#include "../Render/Resources/ShadowResources.h"
#include "../Utilities/GameTimer.h"
#include <DirectXMath.h>

inline float DegreesToRadians(float degrees)
{
    return degrees * (DirectX::XM_PI / 180.0f);
}

void UpdateShadowPassCB(const GameTimer& gt, SceneState& state, const ShadowResources* shadowResources, FrameResource* fr)
{
    auto currPassCB = fr->PassCB.get();

    int lightIndex = 0;
    for (int i = 0; i < (int)state.lights.DirectionalLights.size(); i++, lightIndex++)
    {
        DirectX::XMMATRIX view = XMLoadFloat4x4(&state.lights.LightTransforms[lightIndex].ViewMatrix);
        DirectX::XMMATRIX proj = XMLoadFloat4x4(&state.lights.LightTransforms[lightIndex].ProjectionMatrix);

        DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
        DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
        DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

        UINT w = shadowResources->shadowMaps[lightIndex]->Width();
        UINT h = shadowResources->shadowMaps[lightIndex]->Height();

        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvView, XMMatrixTranspose(invView));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].Proj, XMMatrixTranspose(proj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvProj, XMMatrixTranspose(invProj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].ViewProj, XMMatrixTranspose(viewProj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvViewProj, XMMatrixTranspose(invViewProj));
        state.shadowPassCBs[lightIndex].EyePosW = state.lights.DirectionalLights[i].Position;
        state.shadowPassCBs[lightIndex].RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
        state.shadowPassCBs[lightIndex].InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
        state.shadowPassCBs[lightIndex].NearZ = state.lights.LightTransforms[lightIndex].NearZ;
        state.shadowPassCBs[lightIndex].FarZ = state.lights.LightTransforms[lightIndex].FarZ;

        currPassCB->CopyData(lightIndex + 1, state.shadowPassCBs[lightIndex]);
    }

    for (int i = 0; i < (int)state.lights.SpotLights.size(); i++, lightIndex++)
    {
        DirectX::XMMATRIX view = XMLoadFloat4x4(&state.lights.LightTransforms[lightIndex].ViewMatrix);
        DirectX::XMMATRIX proj = XMLoadFloat4x4(&state.lights.LightTransforms[lightIndex].ProjectionMatrix);

        DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
        DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
        DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

        UINT w = shadowResources->shadowMaps[lightIndex]->Width();
        UINT h = shadowResources->shadowMaps[lightIndex]->Height();

        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvView, XMMatrixTranspose(invView));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].Proj, XMMatrixTranspose(proj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvProj, XMMatrixTranspose(invProj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].ViewProj, XMMatrixTranspose(viewProj));
        XMStoreFloat4x4(&state.shadowPassCBs[lightIndex].InvViewProj, XMMatrixTranspose(invViewProj));
        state.shadowPassCBs[lightIndex].EyePosW = state.lights.SpotLights[i].Position;
        state.shadowPassCBs[lightIndex].RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
        state.shadowPassCBs[lightIndex].InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
        state.shadowPassCBs[lightIndex].NearZ = state.lights.LightTransforms[lightIndex].NearZ;
        state.shadowPassCBs[lightIndex].FarZ = state.lights.LightTransforms[lightIndex].FarZ;

        currPassCB->CopyData(lightIndex + 1, state.shadowPassCBs[lightIndex]);
    }
}

void UpdateShadowTransform(const GameTimer& gt, SceneState& state)
{
    float bounds = -2.0f * state.sceneBounds.Radius;

    int lightIndex = 0;
    for (int i = 0; i < (int)state.lights.DirectionalLights.size(); i++, lightIndex++)
    {
        DirectX::XMFLOAT3 fauxPosition = DirectX::XMFLOAT3(
            bounds * state.mainPassCB.Lights[lightIndex].Direction.x,
            bounds * state.mainPassCB.Lights[lightIndex].Direction.y,
            bounds * state.mainPassCB.Lights[lightIndex].Direction.z);
        DirectX::XMVECTOR lightPos = XMLoadFloat3(&fauxPosition);
        DirectX::XMVECTOR targetPos = XMLoadFloat3(&state.sceneBounds.Center);
        DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);
        XMStoreFloat3(&state.lights.DirectionalLights[i].Position, lightPos);

        DirectX::XMFLOAT3 sphereCenterLS;
        XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

        float l = sphereCenterLS.x - state.sceneBounds.Radius;
        float b = sphereCenterLS.y - state.sceneBounds.Radius;
        float n = sphereCenterLS.z - state.sceneBounds.Radius;
        float r = sphereCenterLS.x + state.sceneBounds.Radius;
        float t = sphereCenterLS.y + state.sceneBounds.Radius;
        float f = sphereCenterLS.z + state.sceneBounds.Radius;

        if (n < 0.1f) n = 0.1f;

        state.lights.LightTransforms[lightIndex].NearZ = n;
        state.lights.LightTransforms[lightIndex].FarZ = f;

        DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

        DirectX::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

        DirectX::XMMATRIX S = lightView * lightProj * T;
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ViewMatrix, lightView);
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ProjectionMatrix, lightProj);
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ViewProjectionMatrix, S);
    }

    for (int i = 0; i < (int)state.lights.SpotLights.size(); i++, lightIndex++)
    {
        DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&state.mainPassCB.Lights[lightIndex].Direction);
        DirectX::XMVECTOR lightPos = XMLoadFloat3(&state.mainPassCB.Lights[lightIndex].Position);

        DirectX::XMVECTOR lightUp = XMLoadFloat3(&up);
        DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, DirectX::XMVectorAdd(lightPos, DirectX::XMVector3Normalize(lightDir)), lightUp);

        state.lights.LightTransforms[lightIndex].NearZ = state.mainPassCB.Lights[lightIndex].FalloffStart;
        state.lights.LightTransforms[lightIndex].FarZ = state.mainPassCB.Lights[lightIndex].FalloffEnd;

        DirectX::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

        float fov = DegreesToRadians(state.mainPassCB.Lights[lightIndex].OuterConeAngle * 2.0f);
        DirectX::XMMATRIX lightProj = DirectX::XMMatrixPerspectiveFovLH(fov, 1.0f, state.lights.LightTransforms[lightIndex].NearZ, state.lights.LightTransforms[lightIndex].FarZ);

        DirectX::XMMATRIX S = lightView * lightProj * T;
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ViewMatrix, lightView);
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ProjectionMatrix, lightProj);
        XMStoreFloat4x4(&state.lights.LightTransforms[lightIndex].ViewProjectionMatrix, S);
    }
}
