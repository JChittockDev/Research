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

    void BuildDescriptors(ID3D12DescriptorHeap* rtvHeap, ID3D12DescriptorHeap* srvHeap, UINT rtvDescriptorSize, UINT srvDescriptorSize);

    void RebuildDescriptors();

	void OnResize(UINT newWidth, UINT newHeight);


    void BuildResources();

    Microsoft::WRL::ComPtr<ID3D12Resource> GetPosition() const { return mPosition; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetNormal() const { return mNormal; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAlbedoSpec() const { return mAlbedoSpec; }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionRtv() const { return mhPositionCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalRtv() const { return mhNormalCpuRtv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoSpecRtv() const { return mhAlbedoSpecCpuRtv; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionSrv() const { return mhPositionCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalSrv() const { return mhNormalCpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoSpecSrv() const { return mhAlbedoSpecCpuSrv; }


private:
	ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mGBufferRootSig;
    
    ID3D12PipelineState* mGBufferPso = nullptr;
	 
    Microsoft::WRL::ComPtr<ID3D12Resource> mPosition;
    Microsoft::WRL::ComPtr<ID3D12Resource> mNormal;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAlbedoSpec;


    CD3DX12_CPU_DESCRIPTOR_HANDLE mhPositionCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhPositionCpuRtv;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalCpuRtv;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAlbedoSpecCpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mhAlbedoSpecCpuRtv;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};