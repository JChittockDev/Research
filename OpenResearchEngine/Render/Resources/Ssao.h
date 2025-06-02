//***************************************************************************************
// Ssao.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#ifndef SSAO_H
#define SSAO_H

#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class Ssao
{
public:

    Ssao(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,  UINT width, UINT height);
    Ssao(const Ssao& rhs) = delete;
    Ssao& operator=(const Ssao& rhs) = delete;
    ~Ssao() = default;

	UINT SsaoWidth()const;
    UINT SsaoHeight()const;

    static const int MaxBlurRadius = 5;

    void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);

    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbient() const { return mAmbient; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbientVerticalBlur() const { return mAmbientVerticalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbientHorizontalBlur() const { return mAmbientHorizontalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetRandomVector() const { return mRandomVector; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientCpuRtv() const { return mhAmbientCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurCpuRtv() const { return mhAmbientVerticalBlurCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurCpuRtv() const { return mhAmbientHorizontalBlurCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRandomVectorCpuRtv() const { return mhRandomVectorCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientCpuSrv() const { return mhAmbientCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurCpuSrv() const { return mhAmbientVerticalBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurCpuSrv() const { return mhAmbientHorizontalBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRandomVectorCpuSrv() const { return mhRandomVectorCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthCpuSrv() const { return mhDepthCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientGpuRtv() const { return mhAmbientGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurRtv() const { return mhAmbientVerticalBlurGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurRtv() const { return mhAmbientHorizontalBlurGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuRtv() const { return mhRandomVectorGpuRtv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientGpuSrv() const { return mhAmbientGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurGpuSrv() const { return mhAmbientVerticalBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurGpuSrv() const { return mhAmbientHorizontalBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuSrv() const { return mhRandomVectorGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv() const { return mhDepthGpuSrv; }

    void BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);

	void OnResize(UINT newWidth, UINT newHeight);
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);
 
	void BuildOffsetVectors();

    std::vector<float> CalcGaussWeights(float sigma);

    void CreateSsaoTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSsaoRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateSsaoSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
    void BuildResources();


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVector;
	Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorUploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbient;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbientVerticalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbientHorizontalBlur;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhRandomVectorCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhRandomVectorCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientVerticalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientVerticalBlurCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientHorizontalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientHorizontalBlurCpuRtv;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhDepthCpuSrv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhRandomVectorGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhRandomVectorGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientVerticalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientVerticalBlurGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientHorizontalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientHorizontalBlurGpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhDepthGpuSrv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

    DirectX::XMFLOAT4 mOffsets[14];

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};

#endif // SSAO_H