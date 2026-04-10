#include "SsaoPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SsaoPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kAmbientResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientVerticalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	
    SetCpuDescriptorHandle(kAmbientResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kRandomVectorResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kDepthResource, "SRV", cpuSrvHandle, srvDescriptorSize);

	SetGpuDescriptorHandle(kAmbientResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kRandomVectorResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kDepthResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void SsaoPassResource::RebuildDescriptors()
{
    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientResource), GetCpuDescriptorHandle(kAmbientResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientResource), GetCpuDescriptorHandle(kAmbientResource, "SRV"));

    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientVerticalBlurResource), GetCpuDescriptorHandle(kAmbientVerticalBlurResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientVerticalBlurResource), GetCpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV"));

    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientHorizontalBlurResource), GetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientHorizontalBlurResource), GetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV"));

    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kRandomVectorResource), GetCpuDescriptorHandle(kRandomVectorResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, GetResource(kDepthResource), GetCpuDescriptorHandle(kDepthResource, "SRV"));
}

void SsaoPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientVerticalBlurResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientHorizontalBlurResource));
    if (!GetResource(kRandomVectorResource))
        CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kRandomVectorResource),
					&GetResource(kRandomVectorUploadResource), randomVectors, RandomVecHeight, RandomVecWidth);

	SetResourceState(kAmbientResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
