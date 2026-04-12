#include "RadiancePassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void RadiancePassResource::Build()
{
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kDiffuseReflectanceResource));
	CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kSpecularReflectanceResource));

	SetResourceState(kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	SetResourceState(kSpecularReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
