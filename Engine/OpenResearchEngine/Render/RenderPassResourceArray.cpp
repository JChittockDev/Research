#include "RenderPassResourceArray.h"

RenderPassResourceArray::RenderPassResourceArray(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height, UINT divisor) : md3dDevice(device), commandList(cmdList), renderWidth(width), renderHeight(height), renderDivisor(divisor)
{
}

void RenderPassResourceArray::OnResize(UINT width, UINT height, UINT divisor)
{
	if (renderWidth != width || renderHeight != height || renderDivisor != divisor)
	{
		renderWidth  = width;
		renderHeight = height;
		renderDivisor = divisor;

		for (auto& resource : resources)
		{
			resource->OnResize(width, height, divisor);
		}
	}
}


CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassResourceArray::GetCpuDescriptorHandleStart(const std::string& type)
{
	if (type == "RTV")
		return resources[0]->GetCpuDescriptorHandleStart("RTV");
	else if (type == "SRV")
		return resources[0]->GetCpuDescriptorHandleStart("SRV");
	else if (type == "DSV")
		return resources[0]->GetCpuDescriptorHandleStart("DSV");
	else
		return CD3DX12_CPU_DESCRIPTOR_HANDLE();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassResourceArray::GetGpuDescriptorHandleStart(const std::string& type)
{
	if (type == "RTV")
		return resources[0]->GetGpuDescriptorHandleStart("RTV");
	else if (type == "SRV")
		return resources[0]->GetGpuDescriptorHandleStart("SRV");
	else if (type == "DSV")
		return resources[0]->GetGpuDescriptorHandleStart("DSV");
	else
		return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}