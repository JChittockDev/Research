#pragma once

#include "../RenderPassResource.h"
 
class SssPassResource : public RenderPassResource
{
public:

    static constexpr const char* kSssResource = "Sss";
    static constexpr const char* kSssBlurResource = "SssBlur";
    static constexpr const char* kRandomVectorResource = "RandomVector";
    static constexpr const char* kRandomVectorUploadResource = "RandomVectorUpload";
    static constexpr const char* kDepthResource = "Depth";

	static constexpr const int RandomVecHeight = 512;
	static constexpr const int RandomVecWidth = 512;

	SssPassResource::SssPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		Math::GenerateRandomVectors(RandomVecHeight, RandomVecWidth, randomVectors);
		Math::GenerateBoxVectors(offsetVectors);
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void RebuildDescriptors() override;

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;

	void Build() override;

	void SetDepthStencilBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer) { SetResource(kDepthResource, depthStencilBuffer); };

	const DirectX::XMFLOAT4* GetOffsetVectors(DirectX::XMFLOAT4 inputVectors[14]) const { return offsetVectors; }
	const std::vector<DirectX::XMFLOAT4>& GetRandomVectors() const { return randomVectors; }

private:
	DirectX::XMFLOAT4   offsetVectors[14];
	std::vector<DirectX::XMFLOAT4> randomVectors;
};