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

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle(gBufferRtvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle(gBufferSrvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle(gBufferSrvHeap.Get()->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle(mDsvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    mGBuffer = std::make_unique<GBuffer>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mGBuffer->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mRenderTextures = std::make_unique<RenderTextures>(md3dDevice.Get());
    mRenderTextures->BuildDescriptors(cpuSrvHandle, gpuSrvHandle, mCbvSrvUavDescriptorSize, mTextureData, mTextures);

    mShadowResources = std::make_unique<ShadowResources>(md3dDevice.Get());
    mShadowResources->BuildDescriptors(dynamicLights.GetNumLights(), cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mDsvDescriptorSize);

    mSsaoMap->BuildDescriptors(mDepthStencilBuffer.Get(), GetCpuSrv(mLayoutIndicies["mSsaoHeapIndex"].first), GetGpuSrv(mLayoutIndicies["mSsaoHeapIndex"].first), GetRtv(SwapChainBufferCount), mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

}