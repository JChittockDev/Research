#include "SsgiPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SsgiPassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kGiResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kGiVerticalBlurResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM,  0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kGiHorizontalBlurResource));
	if (!GetResource(kRandomVectorResource))
		CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kRandomVectorResource),
		&GetResource(kRandomVectorUploadResource), randomVectors, RandomVecHeight, RandomVecWidth, D3D12_RESOURCE_STATE_COPY_DEST);

	SetResourceState(kGiResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kGiVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kGiHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}