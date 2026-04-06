#pragma once
#include "IRenderPassResource.h"

class RenderPassResource : public IRenderPassResource
{
public:
	RenderPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	RenderPassResource(const RenderPassResource& rhs) = delete;
	RenderPassResource& operator=(const RenderPassResource& rhs) = delete;
	~RenderPassResource() = default;
	
	void CreateTexture(const DXGI_FORMAT& format, const DirectX::XMFLOAT4& clearColor, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, 
							Microsoft::WRL::ComPtr<ID3D12Resource>* uploadTexture = nullptr, std::vector<DirectX::XMFLOAT4>* data = nullptr, const UINT& widthOverride = 0, const UINT& heightOverride = 0) override;
	void CreateRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle) override;
	void CreateSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) override;

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const std::string& name, const std::string& type);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSRVDescriptorHandle(const std::string& name);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRTVDescriptorHandle(const std::string& name);

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const std::string& name, const std::string& type);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSRVDescriptorHandle(const std::string& name);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRTVDescriptorHandle(const std::string& name);
	
	void SetCpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const UINT& size);
	void SetGpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_GPU_DESCRIPTOR_HANDLE& handle, const UINT& size);

	void SetResource(const std::string& name, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource(const std::string& name);

	UINT GetRenderWidth() const { return renderWidth; }
	void SetRenderWidth(UINT val) { renderWidth = val; }
	
	UINT GetRenderHeight() const { return renderHeight; }
	void SetRenderHeight(UINT val) { renderHeight = val; }
	
	D3D12_VIEWPORT GetViewport() override { return viewport; }
	void SetViewport(D3D12_VIEWPORT val) override { viewport = val; }

	D3D12_RECT GetScissorRect() override { return scissorRect; }
	void SetScissorRect(D3D12_RECT val) override { scissorRect = val; }

	D3D12_RESOURCE_STATES GetResourceState(const std::string& name);
	void SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state);

private:
	ID3D12Device* d3dDevice;
	ID3D12GraphicsCommandList* commandList;

	UINT renderWidth = 0;
	UINT renderHeight = 0;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	std::unordered_map<std::string, CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuSRVDescriptorHandles;
	std::unordered_map<std::string, CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuSRVDescriptorHandles;
	
	std::unordered_map<std::string, CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuRTVDescriptorHandles;
	std::unordered_map<std::string, CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuRTVDescriptorHandles;
	
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>> resources;
	std::unordered_map<std::string, D3D12_RESOURCE_STATES> states;
};
