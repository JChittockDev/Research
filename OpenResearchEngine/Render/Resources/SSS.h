#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class SSS
{
public:

    SSS(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,  UINT width, UINT height);
    SSS(const SSS& rhs) = delete;
    SSS& operator=(const SSS& rhs) = delete;
    ~SSS() = default;

	UINT SSSWidth()const;
    UINT SSSHeight()const;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSS() const { return mSSS; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSSBlur() const { return mSSSBlur; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSCpuRtv() const { return mhSSSCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSBlurCpuRtv() const { return mhSSSBlurCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSCpuSrv() const { return mhSSSCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSVBlurCpuSrv() const { return mhSSSBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthCpuSrv() const { return mhDepthCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSGpuRtv() const { return mhSSSGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSBlurRtv() const { return mhSSSBlurGpuRtv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSGpuSrv() const { return mhSSSGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSBlurGpuSrv() const { return mhSSSBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv() const { return mhDepthGpuSrv; }

    void BuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);
    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);

	void OnResize(UINT newWidth, UINT newHeight);

    void CreateSSSTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSSSRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateSSSSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
    void BuildResources();


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSS;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSSBlur;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhDepthCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSBlurCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhDepthGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSBlurGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};