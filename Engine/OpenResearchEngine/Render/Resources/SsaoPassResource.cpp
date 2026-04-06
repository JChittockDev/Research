#include "SsaoPassResource.h"
#include <random>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void SsaoPassResource::OnResize(UINT width, UINT height)
{
	if (GetRenderWidth() != width || GetRenderHeight() != height)
	{
		SetRenderWidth(width);
		SetRenderHeight(height);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = width / 2;
		viewport.Height = height / 2;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = { 0, 0, (int)width / 2, (int)height / 2 };

		SetViewport(viewport);
		SetScissorRect(scissorRect);

		Build();
	}
}

void SsaoPassResource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize)
{
	SetCpuDescriptorHandle(kAmbientResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientVerticalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "RTV", cpuRtvHandle, rtvDescriptorSize);
	
    SetCpuDescriptorHandle(kAmbientResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kRandomVectorResource, "SRV", cpuSrvHandle, srvDescriptorSize);
    SetCpuDescriptorHandle(kDepthResource, "SRV", cpuSrvHandle, srvDescriptorSize);

	SetGpuDescriptorHandle(kAmbientResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kRandomVectorResource, "SRV", gpuSrvHandle, srvDescriptorSize);
	SetGpuDescriptorHandle(kDepthResource, "SRV", gpuSrvHandle, srvDescriptorSize);

	RebuildDescriptors();
}

void SsaoPassResource::RebuildDescriptors()
{
    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientResource), GetCpuDescriptorHandle(kAmbientResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientResource), GetCpuDescriptorHandle(kAmbientResource, "SRV"));

    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientVerticalBlurResource), GetCpuDescriptorHandle(kAmbientVerticalBlurResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientVerticalBlurResource), GetCpuDescriptorHandle(kAmbientVerticalBlurResource, "SRV"));

    CreateRTV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientHorizontalBlurResource), GetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "RTV"));
    CreateSRV(DXGI_FORMAT_R16_UNORM, GetResource(kAmbientHorizontalBlurResource), GetCpuDescriptorHandle(kAmbientHorizontalBlurResource, "SRV"));

    CreateSRV(DXGI_FORMAT_R8G8B8A8_UNORM, GetResource(kRandomVectorResource), GetCpuDescriptorHandle(kRandomVectorResource, "SRV"));
    CreateSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, GetResource(kDepthResource), GetCpuDescriptorHandle(kDepthResource, "SRV"));
}

std::vector<DirectX::XMFLOAT4> SsaoPassResource::GenerateRandomVectors(const UINT& width, const UINT& height)
{
	std::vector<XMFLOAT4> initData(width * height);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	for (UINT i = 0; i < height; ++i)
	{
		for (UINT j = 0; j < width; ++j)
		{
			UINT index = i * width + j;

			initData[index] = XMFLOAT4(
				dis(gen), // R: for radius sampling
				dis(gen), // G: for disk sampling X
				dis(gen), // B: for disk sampling Y  
				dis(gen)  // A: extra random value for future use
			);
		}
	}

	return initData;
}

void SsaoPassResource::Build()
{
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientVerticalBlurResource));
    CreateTexture(DXGI_FORMAT_R16_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f }, GetResource(kAmbientHorizontalBlurResource));

    CreateTexture(DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f }, GetResource(kRandomVectorResource), &GetResource(kRandomVectorUploadResource), &GenerateRandomVectors(RandomVecHeight, RandomVecWidth), RandomVecHeight, RandomVecWidth);

	SetResourceState(kAmbientResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kAmbientHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
    SetResourceState(kRandomVectorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void SsaoPassResource::GetOffsetVectors(DirectX::XMFLOAT4 offsets[14])
{
    std::copy(&mOffsets[0], &mOffsets[14], &offsets[0]);
}
 
void SsaoPassResource::BuildOffsetVectors()
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

std::vector<float> SsaoPassResource::CalcGaussWeights(float sigma)
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
