#include "../EngineApp.h"

void EngineApp::SetRenderPassResources()
{
    mShadowPassCBs.resize(dynamicLights.GetNumLights());
    mRadianceCBs.resize(dynamicLights.GetNumLights());

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle(renderPassRtvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle(renderPassSrvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle(renderPassSrvHeap.Get()->GetGPUDescriptorHandleForHeapStart());
   
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle(mDsvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    mGBuffer = std::make_unique<GBufferPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
    mGBuffer->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mSsao = std::make_unique<SsaoPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
	mSsao->SetDepthStencilBuffer(GetDepthBuffer());
    mSsao->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mLighting = std::make_unique<LightingPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
    mLighting->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mSsgi = std::make_unique<SsgiPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
    mSsgi->SetDepthStencilBuffer(GetDepthBuffer());
    mSsgi->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mSss = std::make_unique<SssPassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
    mSss->SetDepthStencilBuffer(GetDepthBuffer());
    mSss->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mComposite = std::make_unique<CompositePassResource>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 2);
    mComposite->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mRadianceResourceArray = std::make_unique<RenderPassResourceArray>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight, 1);
	mRadianceResourceArray->BuildArrayDescriptors<RadiancePassResource>(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, dynamicLights.GetNumLights(), true, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mRenderTextures = std::make_unique<RenderTextures>(md3dDevice.Get());
    mRenderTextures->BuildDescriptors(cpuSrvHandle, gpuSrvHandle, mCbvSrvUavDescriptorSize, mAssets->mTextureData, mAssets->mTextures);

    cpuDsvHandle = cpuDsvHandle.Offset(1, mDsvDescriptorSize);

    mShadowResourceArray = std::make_unique<RenderPassResourceArray>(md3dDevice.Get(), mCommandList.Get(), 2048, 2048, 1);
    mShadowResourceArray->BuildArrayDescriptors<ShadowPassResource>(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, dynamicLights.GetNumLights(), false, mCbvSrvUavDescriptorSize, 0, mDsvDescriptorSize);
}
