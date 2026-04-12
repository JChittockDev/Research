#pragma once

#include "../RenderPassResource.h"
 
class LightingPassResource : public RenderPassResource
{
public:
	static constexpr const char* kLightingResource = "Lighting";

	LightingPassResource::LightingPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		SetResourceTypes({ kLightingResource });
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

    void Build() override;

};