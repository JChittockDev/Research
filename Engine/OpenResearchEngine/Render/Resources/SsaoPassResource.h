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

	static constexpr const int RandomVecHeight = 512;
	static constexpr const int RandomVecWidth = 512;

    SsaoPassResource::SsaoPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		SetResourceTypes({ kAmbientResource, kAmbientVerticalBlurResource, kAmbientHorizontalBlurResource, kRandomVectorResource, kDepthResource });
		Math::GenerateRandomVectors(RandomVecHeight, RandomVecWidth, randomVectors);
		Math::GenerateBoxVectors(offsetVectors);
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void Build() override;

    void SetDepthStencilBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer) { SetResource(kDepthResource, depthStencilBuffer); };

    const DirectX::XMFLOAT4* GetOffsetVectors(DirectX::XMFLOAT4 inputVectors[14]) const { return offsetVectors; }
	const std::vector<DirectX::XMFLOAT4> GetRandomVectors() const { return randomVectors; }

private:
	DirectX::XMFLOAT4   offsetVectors[14];
	std::vector<DirectX::XMFLOAT4> randomVectors;
};