#include "../EngineApp.h"

void EngineApp::UpdateLights(const GameTimer& gt)
{
    mLightRotationAngle += 0.1f * gt.DeltaTime();
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(mLightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&mBaseLightDirections[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&mRotatedLightDirections[i], lightDir);
    }
}