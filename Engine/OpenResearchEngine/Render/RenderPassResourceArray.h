#pragma once
#include "IRenderPassResourceArray.h"
#include "RenderPassResource.h"
#include "../D3D12/D3DUtil.h"
#include <functional>

class RenderPassResourceArray : public IRenderPassResourceArray
{
public:

	RenderPassResourceArray(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor);
	RenderPassResourceArray(const RenderPassResourceArray& rhs) = delete;
	RenderPassResourceArray& operator=(const RenderPassResourceArray& rhs) = delete;
	~RenderPassResourceArray() = default;

	UINT Width()   override { return renderWidth / renderDivisor; }
	UINT Height()  override { return renderHeight / renderDivisor; }
	UINT Divisor() override { return renderDivisor; }

	void OnResize(UINT width, UINT height, UINT divisor) override;

	template<typename T>
	void BuildArrayDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuRtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuSrvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuDsvHandle, const UINT& size, bool sequential = false, UINT srvDescriptorSize = 0, UINT rtvDescriptorSize = 0, UINT dsvDescriptorSize = 0)
	{
		resources.clear();
		resources.reserve(size);

		for (UINT i = 0; i < size; i++)
		{
			resources.push_back(std::make_unique<T>(md3dDevice, commandList, renderWidth, renderHeight, renderDivisor));

			if (!sequential)
			{
				resources[i]->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, srvDescriptorSize, rtvDescriptorSize, dsvDescriptorSize);
			}
		}

		if (sequential)
		{
			auto types = resources[0]->GetResourceTypes();

			for (UINT x = 0; x < types.size(); x++)
			{
				for (UINT i = 0; i < size; i++)
				{
					resources[i]->BuildDescriptorType(types[x], cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, srvDescriptorSize, rtvDescriptorSize, dsvDescriptorSize);
				}
			}
		}
	}
	
    template<typename T>
    T* Get(UINT index) { return static_cast<T*>(resources[index].get()); }
	
	UINT Count() const { return static_cast<UINT>(resources.size()); }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandleStart(const std::string& type);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandleStart(const std::string& type);

private:
	std::vector<std::unique_ptr<RenderPassResource>> resources;
	ID3D12Device* md3dDevice;
	ID3D12GraphicsCommandList* commandList;

	UINT renderWidth  = 0;
	UINT renderHeight = 0;
	UINT renderDivisor = 0;
};
