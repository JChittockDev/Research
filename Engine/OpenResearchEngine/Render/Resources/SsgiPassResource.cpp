#include "SsgiPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SsgiPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kGiResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kGiVerticalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kGiHorizontalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);

	SetCpuDescriptorHandle(kGiResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kGiVerticalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kGiHorizontalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kRandomVectorResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kDepthResource, "SRV", cpuSrvHandle, srvDescriptorSize);

	SetGpuDescriptorHandle(kGiResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kGiVerticalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kGiHorizontalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kRandomVectorResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kDepthResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void SsgiPassResource::RebuildDescriptors()
{
	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiResource), GetCpuDescriptorHandle(kGiResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiResource), GetCpuDescriptorHandle(kGiResource, "SRV"));

	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiVerticalBlurResource), GetCpuDescriptorHandle(kGiVerticalBlurResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiVerticalBlurResource), GetCpuDescriptorHandle(kGiVerticalBlurResource, "SRV"));

	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiHorizontalBlurResource), GetCpuDescriptorHandle(kGiHorizontalBlurResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kGiHorizontalBlurResource), GetCpuDescriptorHandle(kGiHorizontalBlurResource, "SRV"));

	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kRandomVectorResource), GetCpuDescriptorHandle(kRandomVectorResource, "SRV"));
	CreateSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, GetResource(kDepthResource), GetCpuDescriptorHandle(kDepthResource, "SRV"));
}

void SsgiPassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kGiResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kGiVerticalBlurResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kGiHorizontalBlurResource));
	if (!GetResource(kRandomVectorResource))
		CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kRandomVectorResource),
		&GetResource(kRandomVectorUploadResource), randomVectors, RandomVecHeight, RandomVecWidth);

	SetResourceState(kGiResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kGiVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kGiHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}