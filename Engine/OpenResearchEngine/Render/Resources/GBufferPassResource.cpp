#include "GBufferPassResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void GBufferPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { DXGI_FORMAT_R16G16B16A16_FLOAT, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kPositionResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { DXGI_FORMAT_R16G16B16A16_FLOAT, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kNormalResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { DXGI_FORMAT_R16G16B16A16_FLOAT, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kViewNormalResource));
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kAlbedoSpecResource));
    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { DXGI_FORMAT_R8G8B8A8_UNORM, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kReflectionResource));
    CreateTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, { DXGI_FORMAT_R32G32B32A32_FLOAT, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kMaterialIdResource));
    CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, { DXGI_FORMAT_R16G16B16A16_FLOAT, 0.0f, 0.0f, 0.0f, 1.0f }, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, GetResource(kTangentResource));

    SetResourceState(kPositionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kViewNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAlbedoSpecResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kReflectionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kMaterialIdResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kTangentResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

