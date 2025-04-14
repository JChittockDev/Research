#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class GBuffer
{
public:

    GBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    GBuffer(const GBuffer& rhs) = delete;
    GBuffer& operator=(const GBuffer& rhs) = delete;
    ~GBuffer() = default;

	UINT GBufferWidth()const;
    UINT GBufferHeight()const;

    void CreateGBufferTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateGBufferRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);
    void CreateGBufferSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle);

    void RebuildDescriptors();
    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, UINT rtvDescriptorSize, UINT srvDescriptorSize);

	void OnResize(UINT newWidth, UINT newHeight);


    void BuildResources();

    Microsoft::WRL::ComPtr<ID3D12Resource> GetPosition() const { return mPosition; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetNormal() const { return mNormal; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAlbedoSpec() const { return mAlbedoSpec; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialId() const { return mMaterialId; }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionCpuRtv() const { return mhPositionCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalCpuRtv() const { return mhNormalCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoSpecCpuRtv() const { return mhAlbedoSpecCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetMaterialIdCpuRtv() const { return mhMaterialIdCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionCpuSrv() const { return mhPositionCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalCpuSrv() const { return mhNormalCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoSpecCpuSrv() const { return mhAlbedoSpecCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetMaterialIdCpuSrv() const { return mhMaterialIdCpuSrv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetPositionGpuRtv() const { return mhPositionGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetNormalGpuRtv() const { return mhNormalGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoSpecGpuRtv() const { return mhAlbedoSpecGpuRtv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetMaterialIdGpuRtv() const { return mhMaterialIdGpuRtv; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetPositionGpuSrv() const { return mhPositionGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetNormalGpuSrv() const { return mhNormalGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoSpecGpuSrv() const { return mhAlbedoSpecGpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetMaterialIdGpuSrv() const { return mhMaterialIdGpuSrv; }


private:
	ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mGBufferRootSig;
    
    ID3D12PipelineState* mGBufferPso = nullptr;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mPosition;
    Microsoft::WRL::ComPtr<ID3D12Resource> mNormal;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAlbedoSpec;
    Microsoft::WRL::ComPtr<ID3D12Resource> mMaterialId;


    CD3DX12_CPU_DESCRIPTOR_HANDLE mhPositionCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhPositionCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAlbedoSpecCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAlbedoSpecCpuRtv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhMaterialIdCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhMaterialIdCpuRtv;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mhPositionGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhPositionGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhNormalGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhNormalGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAlbedoSpecGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhAlbedoSpecGpuRtv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhMaterialIdGpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mhMaterialIdGpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};