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
    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSSVerticalBlur() const { return mSSSVerticalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSSHorizontalBlur() const { return mSSSHorizontalBlur; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSCpuRtv() const { return mhSSSCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSVerticalBlurCpuRtv() const { return mhSSSVerticalBlurCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSHorizontalBlurCpuRtv() const { return mhSSSHorizontalBlurCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSCpuSrv() const { return mhSSSCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSVerticalBlurCpuSrv() const { return mhSSSVerticalBlurCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSHorizontalBlurCpuSrv() const { return mhSSSHorizontalBlurCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSGpuRtv() const { return mhSSSGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSVerticalBlurRtv() const { return mhSSSVerticalBlurGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSHorizontalBlurRtv() const { return mhSSSHorizontalBlurGpuRtv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSGpuSrv() const { return mhSSSGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSVerticalBlurGpuSrv() const { return mhSSSVerticalBlurGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSHorizontalBlurGpuSrv() const { return mhSSSHorizontalBlurGpuSrv; }

    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);
    void RebuildDescriptors();

	void OnResize(UINT newWidth, UINT newHeight);

    void CreateSSSTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSSSRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateSSSSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);
    void BuildResources();


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSS;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSSVerticalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSSHorizontalBlur;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSVerticalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSVerticalBlurCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSHorizontalBlurCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSSSHorizontalBlurCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSVerticalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSVerticalBlurGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSHorizontalBlurGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSSSHorizontalBlurGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};