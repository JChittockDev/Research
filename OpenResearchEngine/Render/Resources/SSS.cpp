//***************************************************************************************
// SSS.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "SSS.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

SSS::SSS(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height)
{
    md3dDevice = device;
    OnResize(width, height);
}

UINT SSS::SSSWidth()const
{
    return mRenderTargetWidth / 2;
}

UINT SSS::SSSHeight()const
{
    return mRenderTargetHeight / 2;
}

void SSS::BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
    mhSSSCpuRtv = cpuRtvHandle;
    mhSSSBlurCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);

    mhSSSCpuSrv = cpuSrvHandle;
    mhSSSBlurCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhDepthCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);

    mhSSSGpuSrv = gpuSrvHandle;
    mhSSSBlurGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhDepthGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);

    cpuRtvHandle = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    cpuSrvHandle = cpuSrvHandle.Offset(1, srvDescriptorSize);
    gpuSrvHandle = gpuSrvHandle.Offset(1, srvDescriptorSize);

    RebuildDescriptors(depthStencilBuffer);
}

void SSS::CreateSSSRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void SSS::CreateSSSSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle)
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

void SSS::CreateSSSTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
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

void SSS::RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CreateSSSRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSS, mhSSSCpuRtv);
    CreateSSSSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSS, mhSSSCpuSrv);

    CreateSSSRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSSBlur, mhSSSBlurCpuRtv);
    CreateSSSSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSSBlur, mhSSSBlurCpuSrv);

    CreateSSSSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, depthStencilBuffer, mhDepthCpuSrv);
}


void SSS::BuildResources()
{
    CreateSSSTexture(DXGI_FORMAT_R8G8B8A8_UNORM, mSSS);
    CreateSSSTexture(DXGI_FORMAT_R8G8B8A8_UNORM, mSSSBlur);
}


void SSS::OnResize(UINT newWidth, UINT newHeight)
{
    if (mRenderTargetWidth != newWidth || mRenderTargetHeight != newHeight)
    {
        mRenderTargetWidth = newWidth;
        mRenderTargetHeight = newHeight;

        // We render to ambient map at half the resolution.
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