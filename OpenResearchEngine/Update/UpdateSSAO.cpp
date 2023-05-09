#include "../EngineApp.h"

void EngineApp::UpdateSsaoCB(const GameTimer& gt)
{
    SsaoConstants ssaoCB;

    DirectX::XMMATRIX P = mCamera.GetProj();

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    ssaoCB.Proj = mMainPassCB.Proj;
    ssaoCB.InvProj = mMainPassCB.InvProj;
    XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

    mSsaoMap->GetOffsetVectors(ssaoCB.OffsetVectors);

    auto blurWeights = mSsaoMap->CalcGaussWeights(2.5f);
    ssaoCB.BlurWeights[0] = DirectX::XMFLOAT4(&blurWeights[0]);
    ssaoCB.BlurWeights[1] = DirectX::XMFLOAT4(&blurWeights[4]);
    ssaoCB.BlurWeights[2] = DirectX::XMFLOAT4(&blurWeights[8]);

    ssaoCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mSsaoMap->SsaoMapWidth(), 1.0f / mSsaoMap->SsaoMapHeight());

    // Coordinates given in view space.
    ssaoCB.OcclusionRadius = 0.5f;
    ssaoCB.OcclusionFadeStart = 0.2f;
    ssaoCB.OcclusionFadeEnd = 2.0f;
    ssaoCB.SurfaceEpsilon = 0.05f;

    auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
    currSsaoCB->CopyData(0, ssaoCB);
}