#include "ShadowPassResource.h"

void ShadowPassResource::Build()
{
	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	CreateTexture(DXGI_FORMAT_R24G8_TYPELESS, optClear, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, GetResource(kShadowResource), nullptr, {}, 0, 0, D3D12_RESOURCE_STATE_GENERIC_READ);
	ForceSyncState(kShadowResource, D3D12_RESOURCE_STATE_GENERIC_READ);
}