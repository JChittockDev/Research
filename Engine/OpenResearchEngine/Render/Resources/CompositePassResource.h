#pragma once

#include "../RenderPassResource.h"
 
class CompositePassResource : public RenderPassResource
{
public:
	static constexpr const char* kCompositeResource = "Composite";

	CompositePassResource::CompositePassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height) : RenderPassResource(device, cmdList)
	{
		OnResize(width, height);
	}

	UINT Width() override { return GetRenderWidth() / 2; };
	UINT Height() override { return GetRenderHeight() / 2; };

	void OnResize(UINT width, UINT height) override;

	void RebuildDescriptors() override;
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;

	void Build() override;
};