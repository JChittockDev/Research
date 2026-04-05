#pragma once

#include "../RenderPassResource.h"
 
class RadiancePassResource : public RenderPassResource
{
public:
	static constexpr const char* kDiffuseReflectanceResource = "DiffuseReflectance";
	static constexpr const char* kSpecularReflectanceResource = "SpecularReflectance";

    RadiancePassResource::RadiancePassResource(ID3D12Device* device, UINT width, UINT height) : RenderPassResource(device)
	{
		OnResize(width, height);
	}

	UINT Width() override { return GetRenderWidth() / 2; };
	UINT Height() override { return GetRenderHeight() / 2; };

	void OnResize(UINT width, UINT height) override;

	void RebuildDiffuseDescriptors();
	void RebuildSpecularDescriptors();
	
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;
	void RebuildDescriptors() override;
	void BuildDiffuseDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);
	void BuildSpecularDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void Build() override;
};