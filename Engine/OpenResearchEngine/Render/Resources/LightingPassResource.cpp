#include "LightingPassResource.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void LightingPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f}, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kLightingResource));
	SetResourceState(kLightingResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

