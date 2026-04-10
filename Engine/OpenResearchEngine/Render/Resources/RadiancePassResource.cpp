#include "RadiancePassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void RadiancePassResource::BuildDiffuseDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kDiffuseReflectanceResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kDiffuseReflectanceResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kDiffuseReflectanceResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDiffuseDescriptors();
}

void RadiancePassResource::BuildSpecularDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kSpecularReflectanceResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kSpecularReflectanceResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kSpecularReflectanceResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildSpecularDescriptors();
}

void RadiancePassResource::RebuildDiffuseDescriptors()
{
    CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kDiffuseReflectanceResource), GetCpuDescriptorHandle(kDiffuseReflectanceResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kDiffuseReflectanceResource), GetCpuDescriptorHandle(kDiffuseReflectanceResource, "SRV"));
}

void RadiancePassResource::RebuildSpecularDescriptors()
{
    CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSpecularReflectanceResource), GetCpuDescriptorHandle(kSpecularReflectanceResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kSpecularReflectanceResource), GetCpuDescriptorHandle(kSpecularReflectanceResource, "SRV"));
}

void RadiancePassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	BuildDiffuseDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, rtvDescriptorSize, srvDescriptorSize);
	BuildSpecularDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, rtvDescriptorSize, srvDescriptorSize);
}

void RadiancePassResource::RebuildDescriptors()
{
	RebuildDiffuseDescriptors();
	RebuildSpecularDescriptors();
}

void RadiancePassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kDiffuseReflectanceResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kSpecularReflectanceResource));

	SetResourceState(kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kSpecularReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

