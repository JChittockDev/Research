#pragma once
#include "IRenderPassResource.h"
#include "../D3D12/D3DUtil.h"

#include "../Common/DataStructures.h"

class RenderPassResource : public IRenderPassResource
{
public:
	RenderPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	RenderPassResource(const RenderPassResource& rhs) = delete;
	RenderPassResource& operator=(const RenderPassResource& rhs) = delete;
	~RenderPassResource() = default;

	void OnResize(UINT width, UINT height, UINT divisor) override;
	
	void CreateTexture(const DXGI_FORMAT& format, const D3D12_CLEAR_VALUE& clearValue, const D3D12_RESOURCE_FLAGS& flags, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, 
		Microsoft::WRL::ComPtr<ID3D12Resource>* uploadTexture = nullptr, const std::vector<DirectX::XMFLOAT4>& data = {}, const UINT& widthOverride = 0, const UINT& heightOverride = 0, const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_RENDER_TARGET) override;
	void CreateRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle) override;
	void CreateSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) override;
	void CreateDSV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& dsvHandle) override;

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandleStart(const std::string& type);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSRVDescriptorHandle(const std::string& name);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRTVDescriptorHandle(const std::string& name);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDSVDescriptorHandle(const std::string& name);
	bool QueryCpuSRVDescriptorHandle(const std::string& name, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);
	bool QueryCpuRTVDescriptorHandle(const std::string& name, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);
	bool QueryCpuDSVDescriptorHandle(const std::string& name, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandleStart(const std::string& type);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSRVDescriptorHandle(const std::string& name);
	bool QueryGpuSRVDescriptorHandle(const std::string& name, CD3DX12_GPU_DESCRIPTOR_HANDLE& handle);
	
	void SetCpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const UINT& size);
	void SetGpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_GPU_DESCRIPTOR_HANDLE& handle, const UINT& size);

	void RebuildDescriptors() override;

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDsvHandle, UINT srvDescriptorSize = 0, UINT rtvDescriptorSize = 0, UINT dsvDescriptorSize = 0) override;

	void BuildDescriptorType(const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDsvHandle, UINT srvDescriptorSize = 0, UINT rtvDescriptorSize = 0, UINT dsvDescriptorSize = 0) override;

	void RebuildDescriptorType(const std::string& type) override;

	void SetResource(const std::string& name, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource(const std::string& name);

	UINT GetRenderWidth() const { return renderWidth; }
	void SetRenderWidth(UINT val) { renderWidth = val; }
	
	UINT GetRenderHeight() const { return renderHeight; }
	void SetRenderHeight(UINT val) { renderHeight = val; }

	UINT GetRenderDivisor() const { return renderDivisor; }
	void SetRenderDivisor(UINT val) { renderDivisor = val; }
	
	D3D12_VIEWPORT GetViewport() override { return viewport; }
	void SetViewport(D3D12_VIEWPORT val) override { viewport = val; }

	D3D12_RECT GetScissorRect() override { return scissorRect; }
	void SetScissorRect(D3D12_RECT val) override { scissorRect = val; }

	D3D12_RESOURCE_STATES GetResourceState(const std::string& name);
	void SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state);
	void ForceSyncState(const std::string& name, D3D12_RESOURCE_STATES state);
	
	std::vector<const char*> GetResourceTypes() const { return resourceTypes; }
	void SetResourceTypes(std::vector<const char*> val) { resourceTypes = val; }
private:
	ID3D12Device* d3dDevice;
	ID3D12GraphicsCommandList* commandList;

	UINT renderWidth = 0;
	UINT renderHeight = 0;
	UINT renderDivisor = 0;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	DataStructures::TrackedMap<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuSRVDescriptorHandles;
	DataStructures::TrackedMap<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuSRVDescriptorHandles;
	DataStructures::TrackedMap<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuRTVDescriptorHandles;
	DataStructures::TrackedMap<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuDSVDescriptorHandles;
	
	DataStructures::TrackedMap<Microsoft::WRL::ComPtr<ID3D12Resource>> resources;
	std::unordered_map<std::string, D3D12_RESOURCE_STATES> states;
	std::vector<const char*> resourceTypes;

	const D3D12_RESOURCE_STATES defaultState = D3D12_RESOURCE_STATE_RENDER_TARGET;
};
