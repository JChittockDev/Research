#pragma once

#include "../RenderPassResource.h"
 
class GBufferPassResource : public RenderPassResource
{
public:

	static constexpr const char* kPositionResource = "Position";
    static constexpr const char* kNormalResource = "Normal";
    static constexpr const char* kViewNormalResource = "ViewNormal";
    static constexpr const char* kAlbedoSpecResource = "AlbedoSpec";
    static constexpr const char* kReflectionResource = "Reflection";
    static constexpr const char* kMaterialIdResource = "MaterialId";
    static constexpr const char* kTangentResource = "Tangent";

	GBufferPassResource::GBufferPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };


	void RebuildDescriptors() override;
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;

	void Build() override;
};