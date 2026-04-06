#include "CompositePassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void CompositePassResource::OnResize(UINT width, UINT height)
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

void CompositePassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kCompositeResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	SetCpuDescriptorHandle(kCompositeResource, "SRV", cpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kCompositeResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void CompositePassResource::RebuildDescriptors()
{
	CreateRTV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kCompositeResource), GetCpuDescriptorHandle(kCompositeResource, "RTV"));
	CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kCompositeResource), GetCpuDescriptorHandle(kCompositeResource, "SRV"));
}


void CompositePassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kCompositeResource));
	SetResourceState(kCompositeResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

