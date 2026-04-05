#include "LightingPassResource.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void LightingPassResource::OnResize(UINT width, UINT height)
{
	if (GetRenderWidth() != width || GetRenderHeight() != height)
	{
		SetRenderWidth(width);
		SetRenderHeight(height);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = width / 2;
		viewport.Height = height / 2;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		
		D3D12_RECT scissorRect = { 0, 0, (int)width / 2, (int)height / 2 };

		SetViewport(viewport);
		SetScissorRect(scissorRect);

		Build();
	}
}

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

