#pragma once

#include "../RenderPassResource.h"
 
class CompositePassResource : public RenderPassResource
{
public:
	static constexpr const char* kCompositeResource = "Composite";

	CompositePassResource::CompositePassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		SetResourceTypes({ kCompositeResource });
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void Build() override;
};