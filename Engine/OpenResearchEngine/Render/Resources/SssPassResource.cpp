#include "SssPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SssPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kSssResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kSssBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);

	SetCpuDescriptorHandle(kSssResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kSssBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kRandomVectorResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetCpuDescriptorHandle(kDepthResource, "SRV", cpuSrvHandle, srvDescriptorSize);

	SetGpuDescriptorHandle(kSssResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kSssBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kRandomVectorResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kDepthResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void SssPassResource::RebuildDescriptors()
{
	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSssResource), GetCpuDescriptorHandle(kSssResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSssResource), GetCpuDescriptorHandle(kSssResource, "SRV"));

	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSssBlurResource), GetCpuDescriptorHandle(kSssBlurResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSssBlurResource), GetCpuDescriptorHandle(kSssBlurResource, "SRV"));

	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kRandomVectorResource), GetCpuDescriptorHandle(kRandomVectorResource, "SRV"));
	CreateSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, GetResource(kDepthResource), GetCpuDescriptorHandle(kDepthResource, "SRV"));
}

void SssPassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kSssResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kSssBlurResource));
	if (!GetResource(kRandomVectorResource))
		CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kRandomVectorResource),
					&GetResource(kRandomVectorUploadResource), randomVectors, RandomVecHeight, RandomVecWidth);

	SetResourceState(kSssResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kSssBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}