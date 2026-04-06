#pragma once

#include "../RenderPassResource.h"
 
class SsaoPassResource : public RenderPassResource
{
public:
	static constexpr const char* kAmbientResource = "Ambient";
    static constexpr const char* kAmbientVerticalBlurResource = "AmbientVerticalBlur";
    static constexpr const char* kAmbientHorizontalBlurResource = "AmbientHorizontalBlur";
    static constexpr const char* kRandomVectorResource = "RandomVector";
    static constexpr const char* kRandomVectorUploadResource = "RandomVectorUpload";
    static constexpr const char* kDepthResource = "Depth";

    static constexpr const int MaxBlurRadius = 5;
	static constexpr const int RandomVecHeight = 512;
	static constexpr const int RandomVecWidth = 512;

    SsaoPassResource::SsaoPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height) : RenderPassResource(device, cmdList)
	{
		OnResize(width, height);
		BuildOffsetVectors();
	}

	UINT Width() override { return GetRenderWidth() / 2; };
	UINT Height() override { return GetRenderHeight() / 2; };

	void OnResize(UINT width, UINT height) override;

	void RebuildDescriptors() override;
	
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize) override;

	void Build() override;

    void SetDepthStencilBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer) { SetResource(kDepthResource, depthStencilBuffer); };

	std::vector<DirectX::XMFLOAT4> GenerateRandomVectors(const UINT& width, const UINT& height);
	
	void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);
 
	void BuildOffsetVectors();

    std::vector<float> CalcGaussWeights(float sigma);

private:

    DirectX::XMFLOAT4 mOffsets[14];
};