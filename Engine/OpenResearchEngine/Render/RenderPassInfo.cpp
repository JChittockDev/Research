#include "RenderPassInfo.h"

RenderPassInfo::RenderPassInfo(ID3D12Device* device)
{
	md3dDevice = device;
}

void RenderPassInfo::CreateTexture(const DXGI_FORMAT& format, const DirectX::XMFLOAT4& clearColor, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
{
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = GetRenderWidth();
	textureDesc.Height = GetRenderHeight();
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&texture));
}

void RenderPassInfo::CreateRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void RenderPassInfo::CreateSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassInfo::GetCpuDescriptorHandle(const std::string& name, const std::string& type)
{
	if (type == "SRV")
	{
		return GetCpuSRVDescriptorHandle(name);
	}
	else if (type == "RTV")
	{
		return GetCpuRTVDescriptorHandle(name);
	}
	else
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE();
	}
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassInfo::GetCpuSRVDescriptorHandle(const std::string& name)
{
	if (cpuSRVDescriptorHandles.find(name) != cpuSRVDescriptorHandles.end())
	{
		return cpuSRVDescriptorHandles[name];
	}
	else
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE();
	}
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassInfo::GetCpuRTVDescriptorHandle(const std::string& name)
{
	if (cpuRTVDescriptorHandles.find(name) != cpuRTVDescriptorHandles.end())
	{
		return cpuRTVDescriptorHandles[name];
	}
	else
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE();
	}
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassInfo::GetGpuDescriptorHandle(const std::string& name, const std::string& type)
{
	if (type == "SRV")
	{
		return GetGpuSRVDescriptorHandle(name);
	}
	else if (type == "RTV")
	{
		return GetGpuRTVDescriptorHandle(name);
	}
	else
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassInfo::GetGpuSRVDescriptorHandle(const std::string& name)
{
	if (gpuSRVDescriptorHandles.find(name) != gpuSRVDescriptorHandles.end())
	{
		return gpuSRVDescriptorHandles[name];
	}
	else
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}
}

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassInfo::GetGpuRTVDescriptorHandle(const std::string& name)
{
	if (gpuRTVDescriptorHandles.find(name) != gpuRTVDescriptorHandles.end())
	{
		return gpuRTVDescriptorHandles[name];
	}
	else
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}
}

void RenderPassInfo::SetCpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const UINT& size)
{
	if (type == "SRV")
	{
		cpuSRVDescriptorHandles[name] = handle;
		handle = handle.Offset(1, size);
	}
	else if (type == "RTV")
	{
		cpuRTVDescriptorHandles[name] = handle;
		handle = handle.Offset(1, size);
	}
}

void RenderPassInfo::SetGpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_GPU_DESCRIPTOR_HANDLE& handle, const UINT& size)
{
	if (type == "SRV")
	{
		gpuSRVDescriptorHandles[name] = handle;
		handle = handle.Offset(1, size);
	}
	else if (type == "RTV")
	{
		gpuRTVDescriptorHandles[name] = handle;
		handle = handle.Offset(1, size);
	}
}

void RenderPassInfo::SetResource(const std::string& name, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
{
	resources[name] = resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource>& RenderPassInfo::GetResource(const std::string& name)
{
	return resources[name];
}
