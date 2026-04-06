#include "../EngineApp.h"

void EngineApp::SetRenderPassResources()
{
    mShadowPassCBs.resize(dynamicLights.GetNumLights());
    mRadianceCBs.resize(dynamicLights.GetNumLights());

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle(renderPassRtvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle(renderPassSrvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle(renderPassSrvHeap.Get()->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle(mDsvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    mGBuffer = std::make_unique<GBufferPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mGBuffer->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSsao = std::make_unique<SsaoPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
	mSsao->SetDepthStencilBuffer(GetDepthBuffer());
    mSsao->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mLighting = std::make_unique<LightingPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mLighting->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSsgi = std::make_unique<Ssgi>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsgi->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSss = std::make_unique<SSS>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSss->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mComposite = std::make_unique<CompositePassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mComposite->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mRadianceResources = std::make_unique<RadianceResources>(md3dDevice.Get(), mCommandList.Get());
    mRadianceResources->BuildDescriptors(dynamicLights.GetNumLights(), mClientWidth, mClientHeight,
        cpuSrvHandle, gpuSrvHandle, cpuRtvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mRenderTextures = std::make_unique<RenderTextures>(md3dDevice.Get());
    mRenderTextures->BuildDescriptors(cpuSrvHandle, gpuSrvHandle, mCbvSrvUavDescriptorSize,
        mAssets->mTextureData, mAssets->mTextures);

    cpuDsvHandle = cpuDsvHandle.Offset(1, mDsvDescriptorSize);
    mShadowResources = std::make_unique<ShadowResources>(md3dDevice.Get());
    mShadowResources->BuildDescriptors(dynamicLights.GetNumLights(),
        cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mDsvDescriptorSize);
}
