//***************************************************************************************
// Ssgi.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#ifndef SSGI_H
#define SSGI_H

#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class Ssgi
{
public:

    Ssgi(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,  UINT width, UINT height);
    Ssgi(const Ssgi& rhs) = delete;
    Ssgi& operator=(const Ssgi& rhs) = delete;
    ~Ssgi() = default;

	UINT SsgiWidth()const;
    UINT SsgiHeight()const;

    static const int MaxBlurRadius = 5;

    void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);

    Microsoft::WRL::ComPtr<ID3D12Resource> GetGI() const { return mGI; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetGIVerticalBlur() const { return mGIVerticalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetGIHorizontalBlur() const { return mGIHorizontalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetRandomVector() const { return mRandomVector; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetGICpuRtv() const { return mhGICpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIVerticalBlurCpuRtv() const { return mhGIVerticalBlurCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurCpuRtv() const { return mhGIHorizontalBlurCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRandomVectorCpuRtv() const { return mhRandomVectorCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetGICpuSrv() const { return mhGICpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIVerticalBlurCpuSrv() const { return mhGIVerticalBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurCpuSrv() const { return mhGIHorizontalBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRandomVectorCpuSrv() const { return mhRandomVectorCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthCpuSrv() const { return mhDepthCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGIGpuRtv() const { return mhGIGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIVerticalBlurRtv() const { return mhGIVerticalBlurGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurRtv() const { return mhGIHorizontalBlurGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuRtv() const { return mhRandomVectorGpuRtv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGIGpuSrv() const { return mhGIGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIVerticalBlurGpuSrv() const { return mhGIVerticalBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurGpuSrv() const { return mhGIHorizontalBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuSrv() const { return mhRandomVectorGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv() const { return mhDepthGpuSrv; }

    void BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);

	void OnResize(UINT newWidth, UINT newHeight);
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);
 
	void BuildOffsetVectors();

    std::vector<float> CalcGaussWeights(float sigma);

    void CreateSsgiTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSsgiRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateSsgiSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
    void BuildResources();


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVector;
	Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorUploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> mGI;
    Microsoft::WRL::ComPtr<ID3D12Resource> mGIVerticalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mGIHorizontalBlur;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhRandomVectorCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhRandomVectorCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGICpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGICpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGIVerticalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGIVerticalBlurCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGIHorizontalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhGIHorizontalBlurCpuRtv;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhDepthCpuSrv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhRandomVectorGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhRandomVectorGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIVerticalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIVerticalBlurGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIHorizontalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhGIHorizontalBlurGpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhDepthGpuSrv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

    DirectX::XMFLOAT4 mOffsets[14];

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};

#endif // SSAO_H