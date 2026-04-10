#include "LightingPassResource.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void LightingPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
    SetCpuDescriptorHandle(kLightingResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kLightingResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetGpuDescriptorHandle(kLightingResource, "SRV", gpuSrvHandle, srvDescriptorSize);

    RebuildDescriptors();
}

void LightingPassResource::RebuildDescriptors()
{
    CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kLightingResource), GetCpuDescriptorHandle(kLightingResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kLightingResource), GetCpuDescriptorHandle(kLightingResource, "SRV"));
}


void LightingPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 1.0f}, GetResource(kLightingResource));
	SetResourceState(kLightingResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

