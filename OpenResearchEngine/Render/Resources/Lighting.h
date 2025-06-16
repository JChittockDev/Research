#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class Lighting
{
public:

    Lighting(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Lighting(const Lighting& rhs) = delete;
    Lighting& operator=(const Lighting& rhs) = delete;
    ~Lighting() = default;

	UINT LightingWidth()const;
    UINT LightingHeight()const;

    void CreateLightingTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateLightingRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateLightingSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);

    void RebuildDescriptors();
    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void OnResize(UINT newWidth, UINT newHeight);

    void BuildResources();

    Microsoft::WRL::ComPtr<ID3D12Resource> GetLighting() const { return mLighting; }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetLightingCpuRtv() const { return mhLightingCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetLightingCpuSrv() const { return mhLightingCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetLightingGpuRtv() const { return mhLightingGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetLightingGpuSrv() const { return mhLightingGpuSrv; }


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mLighting;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhLightingCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhLightingCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhLightingGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhLightingGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};