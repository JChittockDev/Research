#pragma once

#include "../RenderPassResource.h"

class RadiancePassResource : public RenderPassResource
{
public:
	static constexpr const char* kDiffuseReflectanceResource = "DiffuseReflectance";
	static constexpr const char* kSpecularReflectanceResource = "SpecularReflectance";

	RadiancePassResource::RadiancePassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : RenderPassResource(device, cmdList)
	{
		SetResourceTypes({ kDiffuseReflectanceResource, kSpecularReflectanceResource });
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void Build() override;

};
