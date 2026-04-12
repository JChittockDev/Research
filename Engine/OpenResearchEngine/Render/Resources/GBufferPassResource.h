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
		SetResourceTypes({ kPositionResource, kNormalResource, kViewNormalResource, kAlbedoSpecResource, kReflectionResource, kMaterialIdResource, kTangentResource});
		OnResize(width, height, divisor);
	}

	UINT Width() override { return GetRenderWidth() / GetRenderDivisor(); };
	UINT Height() override { return GetRenderHeight() / GetRenderDivisor(); };
	UINT Divisor() override { return GetRenderDivisor(); };

	void Build() override;
};