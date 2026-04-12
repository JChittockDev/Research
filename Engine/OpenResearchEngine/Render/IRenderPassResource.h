#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "../Common/Structures.h"

class IRenderPassResource
{
public:
	virtual UINT Width() = 0;
	virtual UINT Height() = 0;
	virtual UINT Divisor() = 0;

    virtual void OnResize(UINT newWidth, UINT newHeight, UINT divisor) = 0;

    virtual void Build() = 0;

    virtual void CreateTexture(const DXGI_FORMAT& format, const D3D12_CLEAR_VALUE& clearValue, const D3D12_RESOURCE_FLAGS& flags, Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
                                Microsoft::WRL::ComPtr<ID3D12Resource>* uploadTexture, const std::vector<DirectX::XMFLOAT4>& data, const UINT& widthOverride, const UINT& heightOverride, const D3D12_RESOURCE_STATES& state) = 0;
    virtual void CreateRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle) = 0;
    virtual void CreateSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) = 0;
    virtual void CreateDSV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle) = 0;

    virtual void RebuildDescriptors() = 0;

    virtual void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDsvHandle, UINT srvDescriptorSize, UINT rtvDescriptorSize, UINT dsvDescriptorSize) = 0;

	virtual void BuildDescriptorType(const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDsvHandle, UINT srvDescriptorSize, UINT rtvDescriptorSize, UINT dsvDescriptorSize) = 0;

    virtual void RebuildDescriptorType(const std::string& type) = 0;

    virtual D3D12_VIEWPORT GetViewport() = 0;
    virtual void SetViewport(D3D12_VIEWPORT val) = 0;

    virtual D3D12_RECT GetScissorRect() = 0;
    virtual void SetScissorRect(D3D12_RECT val) = 0;
};
