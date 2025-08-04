#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class Radiance
{
public:

    Radiance(ID3D12Device* device, UINT width, UINT height);
    Radiance(const Radiance& rhs) = delete;
    Radiance& operator=(const Radiance& rhs) = delete;
    ~Radiance() = default;

	UINT RadianceWidth()const;
    UINT RadianceHeight()const;

    void CreateRadianceTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateRadianceRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateRadianceSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);

    void RebuildDiffuseDescriptors();
    void RebuildSpecularDescriptors();
    void BuildDiffuseDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);
    void BuildSpecularDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void OnResize(UINT newWidth, UINT newHeight);

    void BuildResources();

    Microsoft::WRL::ComPtr<ID3D12Resource> GetDiffuseReflectance() const { return mDiffuseReflectance; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetSpecularReflectance() const { return mSpecularReflectance; }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetDiffuseReflectanceCpuRtv() const { return mhDiffuseReflectanceCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDiffuseReflectanceCpuSrv() const { return mhDiffuseReflectanceCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDiffuseReflectanceGpuRtv() const { return mhDiffuseReflectanceGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDiffuseReflectanceGpuSrv() const { return mhDiffuseReflectanceGpuSrv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSpecularReflectanceCpuRtv() const { return mhSpecularReflectanceCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSpecularReflectanceCpuSrv() const { return mhSpecularReflectanceCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSpecularReflectanceGpuRtv() const { return mhSpecularReflectanceGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSpecularReflectanceGpuSrv() const { return mhSpecularReflectanceGpuSrv; }


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mDiffuseReflectance;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSpecularReflectance;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhDiffuseReflectanceCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhDiffuseReflectanceCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSpecularReflectanceCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhSpecularReflectanceCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhDiffuseReflectanceGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhDiffuseReflectanceGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSpecularReflectanceGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhSpecularReflectanceGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};