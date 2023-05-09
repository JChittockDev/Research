#include "../EngineApp.h"

void EngineApp::SetRenderPassResources()
{
    int numLights = dynamicLights.GetNumLights();
    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        mShadowMaps.push_back(std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048));
    }
    mShadowPassCBs.resize(numLights);

    mSsaoMap = std::make_unique<SsaoMap>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsaoMap->SetPSOs(mPSOs["ssao"].Get(), mPSOs["ssaoBlur"].Get());

    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        mShadowMaps[i]->BuildDescriptors(GetCpuSrv(mLayoutIndicies["mShadowMapHeapIndex"].first + i), GetGpuSrv(mLayoutIndicies["mShadowMapHeapIndex"].first + i), GetDsv(i + 1));
    }

    mSsaoMap->BuildDescriptors(mDepthStencilBuffer.Get(), GetCpuSrv(mLayoutIndicies["mSsaoHeapIndexStart"].first), GetGpuSrv(mLayoutIndicies["mSsaoHeapIndexStart"].first), GetRtv(SwapChainBufferCount), mCbvSrvUavDescriptorSize, mRtvDescriptorSize);
}