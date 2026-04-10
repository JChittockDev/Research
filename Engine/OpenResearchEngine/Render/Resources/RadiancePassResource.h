#pragma once

#include "../RenderPassResource.h"
 
class RadiancePassResource : public RenderPassResource
{
public:
	static constexpr const char* kDiffuseReflectanceResource = "DiffuseReflectance";
	static constexpr const char* kSpecularReflectanceResource = "SpecularReflectance";

	RadiancePassResource::RadiancePassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void RebuildDiffuseDescriptors();
	void RebuildSpecularDescriptors();
	void RebuildDescriptors() override;
	
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;
	void BuildDiffuseDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);
	void BuildSpecularDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void Build() override;
};