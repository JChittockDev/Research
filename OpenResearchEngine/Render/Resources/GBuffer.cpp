#include "GBuffer.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

GBuffer::GBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height)
{
    md3dDevice = device;
    OnResize(width, height);
}

UINT GBuffer::GBufferWidth()const
{
    return mRenderTargetWidth / 2;
}

UINT GBuffer::GBufferHeight()const
{
    return mRenderTargetHeight / 2;
}

void GBuffer::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
    mhPositionCpuRtv = cpuRtvHandle;
    mhNormalCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    mhAlbedoSpecCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    mhMaterialIdCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);

    mhPositionCpuSrv = cpuSrvHandle;
    mhNormalCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhAlbedoSpecCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhMaterialIdCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);

    mhPositionGpuSrv = gpuSrvHandle;
    mhNormalGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhAlbedoSpecGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhMaterialIdGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);

    cpuRtvHandle = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    cpuSrvHandle = cpuSrvHandle.Offset(1, srvDescriptorSize);
    gpuSrvHandle = gpuSrvHandle.Offset(1, srvDescriptorSize);



    RebuildDescriptors();
}

void GBuffer::RebuildDescriptors()
{
    CreateGBufferRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, mPosition, mhPositionCpuRtv);
    CreateGBufferRTV(DXGI_FORMAT_R16G16B16A16_FLOAT, mNormal, mhNormalCpuRtv);
    CreateGBufferRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mAlbedoSpec, mhAlbedoSpecCpuRtv);
    CreateGBufferRTV(DXGI_FORMAT_R32_UINT, mMaterialId, mhMaterialIdCpuRtv);

    CreateGBufferSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, mPosition, mhPositionCpuSrv);
    CreateGBufferSRV(DXGI_FORMAT_R16G16B16A16_FLOAT, mNormal, mhNormalCpuSrv);
    CreateGBufferSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mAlbedoSpec, mhAlbedoSpecCpuSrv);
    CreateGBufferSRV(DXGI_FORMAT_R32_UINT, mMaterialId, mhMaterialIdCpuSrv);
}

void GBuffer::CreateGBufferRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void GBuffer::CreateGBufferSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = format;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    md3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);
}

void GBuffer::OnResize(UINT newWidth, UINT newHeight)
{
    if(mRenderTargetWidth != newWidth || mRenderTargetHeight != newHeight)
    {
        mRenderTargetWidth = newWidth;
        mRenderTargetHeight = newHeight;
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = mRenderTargetWidth / 2.0f;
        mViewport.Height = mRenderTargetHeight / 2.0f;
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;
        mScissorRect = { 0, 0, (int)mRenderTargetWidth / 2, (int)mRenderTargetHeight / 2 };

        BuildResources();
    }
}
 
void GBuffer::CreateGBufferTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = mRenderTargetWidth;
    textureDesc.Height = mRenderTargetHeight;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&texture));
}

void GBuffer::BuildResources()
{
    CreateGBufferTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, mPosition);
    CreateGBufferTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, mNormal);
    CreateGBufferTexture(DXGI_FORMAT_R8G8B8A8_UNORM, mAlbedoSpec);
    CreateGBufferTexture(DXGI_FORMAT_R32_UINT, mMaterialId);
}

