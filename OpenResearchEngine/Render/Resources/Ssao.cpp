#include "Ssao.h"
#include <DirectXPackedVector.h>
#include <random>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

Ssao::Ssao(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height)
{
    md3dDevice = device;
    OnResize(width, height);
	BuildOffsetVectors();
	BuildRandomVectorTexture(cmdList);
}

UINT Ssao::SsaoWidth()const
{
    return mRenderTargetWidth / 2;
}

UINT Ssao::SsaoHeight()const
{
    return mRenderTargetHeight / 2;
}

void Ssao::GetOffsetVectors(DirectX::XMFLOAT4 offsets[14])
{
    std::copy(&mOffsets[0], &mOffsets[14], &offsets[0]);
}

void Ssao::BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
    mhAmbientCpuRtv = cpuRtvHandle;
    mhAmbientVerticalBlurCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    mhAmbientHorizontalBlurCpuRtv = cpuRtvHandle.Offset(1, rtvDescriptorSize);

    mhAmbientCpuSrv = cpuSrvHandle;
    mhAmbientVerticalBlurCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhAmbientHorizontalBlurCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhRandomVectorCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);
    mhDepthCpuSrv = cpuSrvHandle.Offset(1, srvDescriptorSize);

    mhAmbientGpuSrv = gpuSrvHandle;
    mhAmbientVerticalBlurGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhAmbientHorizontalBlurGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhRandomVectorGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);
    mhDepthGpuSrv = gpuSrvHandle.Offset(1, srvDescriptorSize);

    cpuRtvHandle = cpuRtvHandle.Offset(1, rtvDescriptorSize);
    cpuSrvHandle = cpuSrvHandle.Offset(1, srvDescriptorSize);
    gpuSrvHandle = gpuSrvHandle.Offset(1, srvDescriptorSize);

    RebuildDescriptors(depthStencilBuffer);
}

void Ssao::CreateSsaoRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void Ssao::CreateSsaoSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle)
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

void Ssao::CreateSsaoTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
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
    clearValue.Color[0] = 1.0f;
    clearValue.Color[1] = 1.0f;
    clearValue.Color[2] = 1.0f;
    clearValue.Color[3] = 1.0f;

    md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&texture));
}

void Ssao::RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbient, mhAmbientCpuRtv);
    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM, mAmbient, mhAmbientCpuSrv);

    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbientVerticalBlur, mhAmbientVerticalBlurCpuRtv);
    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM, mAmbientVerticalBlur, mhAmbientVerticalBlurCpuSrv);

    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbientHorizontalBlur, mhAmbientHorizontalBlurCpuRtv);
    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM, mAmbientHorizontalBlur, mhAmbientHorizontalBlurCpuSrv);

    CreateSsaoSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mRandomVector, mhRandomVectorCpuSrv);
    CreateSsaoSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, depthStencilBuffer, mhDepthCpuSrv);
}


void Ssao::BuildResources()
{
    CreateSsaoTexture(DXGI_FORMAT_R16_UNORM, mAmbient);
    CreateSsaoTexture(DXGI_FORMAT_R16_UNORM, mAmbientVerticalBlur);
    CreateSsaoTexture(DXGI_FORMAT_R16_UNORM, mAmbientHorizontalBlur);
}


void Ssao::OnResize(UINT newWidth, UINT newHeight)
{
    if(mRenderTargetWidth != newWidth || mRenderTargetHeight != newHeight)
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
 

void Ssao::BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList)
{
    constexpr UINT texWidth = 512;
    constexpr UINT texHeight = 512;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = texWidth;
    texDesc.Height = texHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&mRandomVector)));

    // Calculate upload buffer size
    const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mRandomVector.Get(), 0, num2DSubresources);

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(mRandomVectorUploadBuffer.GetAddressOf())));

    std::vector<XMFLOAT4> initData(texWidth * texHeight);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for (UINT i = 0; i < texHeight; ++i)
    {
        for (UINT j = 0; j < texWidth; ++j)
        {
            UINT index = i * texWidth + j;

            initData[index] = XMFLOAT4(
                dis(gen), // R: for radius sampling
                dis(gen), // G: for disk sampling X
                dis(gen), // B: for disk sampling Y  
                dis(gen)  // A: extra random value for future use
            );
        }
    }

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData.data();
    subResourceData.RowPitch = texWidth * sizeof(XMFLOAT4);
    subResourceData.SlicePitch = subResourceData.RowPitch * texHeight;

    UpdateSubresources(cmdList, mRandomVector.Get(), mRandomVectorUploadBuffer.Get(),
        0, 0, num2DSubresources, &subResourceData);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mRandomVector.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
 
void Ssao::BuildOffsetVectors()
{
	mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

    for(int i = 0; i < 14; ++i)
	{
		float s = Math::RandF(0.25f, 1.0f);
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));
		XMStoreFloat4(&mOffsets[i], v);
	}
}

std::vector<float> Ssao::CalcGaussWeights(float sigma)
{
    float twoSigma2 = 2.0f * sigma * sigma;

    // Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
    // For example, for sigma = 3, the width of the bell curve is 
    int blurRadius = (int)ceil(2.0f * sigma);

    assert(blurRadius <= MaxBlurRadius);

    std::vector<float> weights;
    weights.resize(2 * blurRadius + 1);

    float weightSum = 0.0f;

    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float x = (float)i;

        weights[i + blurRadius] = expf(-x * x / twoSigma2);

        weightSum += weights[i + blurRadius];
    }

    // Divide by the sum so all the weights add up to 1.0.
    for (int i = 0; i < weights.size(); ++i)
    {
        weights[i] /= weightSum;
    }

    return weights;
}
