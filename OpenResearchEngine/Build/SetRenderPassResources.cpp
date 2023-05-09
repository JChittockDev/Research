#include "../EngineApp.h"

void EngineApp::SetRenderPassResources()
{
    int numLights = dynamicLights.GetNumLights();
    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        renderCore->shadowMaps.push_back(std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048));
    }
    mShadowPassCBs.resize(numLights);

    renderCore->ssao = std::make_unique<Ssao>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    renderCore->ssao->SetPSOs(mPSOs["ssao"].Get(), mPSOs["ssaoBlur"].Get());

    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        renderCore->shadowMaps[i]->BuildDescriptors(GetCpuSrv(mLayoutIndicies["mShadowMapHeapIndex"].first + i), GetGpuSrv(mLayoutIndicies["mShadowMapHeapIndex"].first + i), GetDsv(i + 1));
    }

    renderCore->ssao->BuildDescriptors(mDepthStencilBuffer.Get(), GetCpuSrv(mLayoutIndicies["mSsaoHeapIndexStart"].first), GetGpuSrv(mLayoutIndicies["mSsaoHeapIndexStart"].first), GetRtv(SwapChainBufferCount), mCbvSrvUavDescriptorSize, mRtvDescriptorSize);
}