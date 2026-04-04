#pragma once

#include "../RenderPassInfo.h"
#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class LightingPassInfo : public RenderPassInfo
{
public:
	static constexpr const char* kLightingResource = "Lighting";

	LightingPassInfo::LightingPassInfo(ID3D12Device* device, UINT width, UINT height) : RenderPassInfo(device)
	{
		OnResize(width, height);
	}

	UINT Width() override { return GetRenderWidth() / 2; };
	UINT Height() override { return GetRenderHeight() / 2;};

	void OnResize(UINT width, UINT height) override;

    void RebuildDescriptors() override;
    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;

    void Build() override;

};