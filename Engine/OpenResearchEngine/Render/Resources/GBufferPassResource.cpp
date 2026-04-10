#include "GBufferPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void GBufferPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kPositionResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kNormalResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kViewNormalResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kAlbedoSpecResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kReflectionResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kMaterialIdResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kTangentResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	
    SetCpuDescriptorHandle(kPositionResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kNormalResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kViewNormalResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kAlbedoSpecResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kReflectionResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kMaterialIdResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kTangentResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	
    SetGpuDescriptorHandle(kPositionResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kNormalResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kViewNormalResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kAlbedoSpecResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kReflectionResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kMaterialIdResource, "SRV", gpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kTangentResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void GBufferPassResource::RebuildDescriptors()
{
	CreateRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kPositionResource), GetCpuDescriptorHandle(kPositionResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kNormalResource), GetCpuDescriptorHandle(kNormalResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kViewNormalResource), GetCpuDescriptorHandle(kViewNormalResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kAlbedoSpecResource), GetCpuDescriptorHandle(kAlbedoSpecResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kReflectionResource), GetCpuDescriptorHandle(kReflectionResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R32G32B32A32_FLOAT, GetResource(kMaterialIdResource), GetCpuDescriptorHandle(kMaterialIdResource, "RTV"));
    CreateRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kTangentResource), GetCpuDescriptorHandle(kTangentResource, "RTV"));

	CreateSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kPositionResource), GetCpuDescriptorHandle(kPositionResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kNormalResource), GetCpuDescriptorHandle(kNormalResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kViewNormalResource), GetCpuDescriptorHandle(kViewNormalResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kAlbedoSpecResource), GetCpuDescriptorHandle(kAlbedoSpecResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kReflectionResource), GetCpuDescriptorHandle(kReflectionResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R32G32B32A32_FLOAT, GetResource(kMaterialIdResource), GetCpuDescriptorHandle(kMaterialIdResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, GetResource(kTangentResource), GetCpuDescriptorHandle(kTangentResource, "SRV"));
}


void GBufferPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kPositionResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kNormalResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kViewNormalResource));
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kAlbedoSpecResource));
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kReflectionResource));
    CreateTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kMaterialIdResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kTangentResource));

    SetResourceState(kPositionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kViewNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAlbedoSpecResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kReflectionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kMaterialIdResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kTangentResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

