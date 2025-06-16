#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class Composite
{
public:

    Composite(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Composite(const Composite& rhs) = delete;
    Composite& operator=(const Composite& rhs) = delete;
    ~Composite() = default;

	UINT CompositeWidth()const;
    UINT CompositeHeight()const;

    void CreateCompositeTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateCompositeRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateCompositeSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);

    void RebuildDescriptors();
    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void OnResize(UINT newWidth, UINT newHeight);

    void BuildResources();

    Microsoft::WRL::ComPtr<ID3D12Resource> GetComposite() const { return mComposite; }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetCompositeCpuRtv() const { return mhCompositeCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetCompositeCpuSrv() const { return mhCompositeCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetCompositeGpuRtv() const { return mhCompositeGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetCompositeGpuSrv() const { return mhCompositeGpuSrv; }


private:
	ID3D12Device* md3dDevice;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mComposite;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhCompositeCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhCompositeCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhCompositeGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhCompositeGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};