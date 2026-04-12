#include "SsaoPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SsaoPassResource::Build()
{

    CreateTexture(DXGI_FORMAT_R16_UNORM, { DXGI_FORMAT_R16_UNORM, 1.0f, 1.0f, 1.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kAmbientResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { DXGI_FORMAT_R16_UNORM, 1.0f, 1.0f, 1.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kAmbientVerticalBlurResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { DXGI_FORMAT_R16_UNORM, 1.0f, 1.0f, 1.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kAmbientHorizontalBlurResource));
    if (!GetResource(kRandomVectorResource))
        CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kRandomVectorResource),
					&GetResource(kRandomVectorUploadResource), randomVectors, RandomVecHeight, RandomVecWidth, D3D12_RESOURCE_STATE_COPY_DEST);

	SetResourceState(kAmbientResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
