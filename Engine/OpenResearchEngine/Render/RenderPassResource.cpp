#include "RenderPassResource.h"

RenderPassResource::RenderPassResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	d3dDevice = device;
	commandList = cmdList;
}

void RenderPassResource::OnResize(UINT width, UINT height, UINT divisor)
{
	if (GetRenderWidth() != width || GetRenderHeight() != height || GetRenderDivisor() != divisor)
	{
		SetRenderWidth(width);
		SetRenderHeight(height);
		SetRenderDivisor(divisor);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = width / divisor;
		viewport.Height = height / divisor;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = { 0, 0, (int)width / divisor, (int)height / divisor };

		SetViewport(viewport);
		SetScissorRect(scissorRect);

		Build();
	}
}

void RenderPassResource::CreateTexture(const DXGI_FORMAT& format, const DirectX::XMFLOAT4& clearColor, Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
											Microsoft::WRL::ComPtr<ID3D12Resource>* uploadTexture, const std::vector<DirectX::XMFLOAT4>& data, const UINT& widthOverride, const UINT& heightOverride)
{
	const UINT width = (widthOverride > 0) ? widthOverride : GetRenderWidth();
	const UINT height = (heightOverride > 0) ? heightOverride : GetRenderHeight();

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = width;
	textureDesc.Height = height;
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

	D3D12_RESOURCE_STATES initState = defaultState;

	if (!data.empty())
	{
		initState = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	ThrowIfFailed(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureDesc, initState, &clearValue, IID_PPV_ARGS(&texture)));

	if (!data.empty() && uploadTexture != nullptr)
	{
		const UINT num2DSubresources = textureDesc.DepthOrArraySize * textureDesc.MipLevels;
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, num2DSubresources);

		ThrowIfFailed(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, 
						&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(uploadTexture->GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = data.data();
		subResourceData.RowPitch = width * sizeof(DirectX::XMFLOAT4);
		subResourceData.SlicePitch = subResourceData.RowPitch * height;

		UpdateSubresources(commandList, texture.Get(), uploadTexture->Get(), 0, 0, num2DSubresources, &subResourceData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, defaultState));
	}
}

void RenderPassResource::CreateRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	d3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void RenderPassResource::CreateSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	d3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassResource::GetCpuDescriptorHandle(const std::string& name, const std::string& type)
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

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassResource::GetCpuSRVDescriptorHandle(const std::string& name)
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

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderPassResource::GetCpuRTVDescriptorHandle(const std::string& name)
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

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassResource::GetGpuDescriptorHandle(const std::string& name, const std::string& type)
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

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassResource::GetGpuSRVDescriptorHandle(const std::string& name)
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

CD3DX12_GPU_DESCRIPTOR_HANDLE RenderPassResource::GetGpuRTVDescriptorHandle(const std::string& name)
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

void RenderPassResource::SetCpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const UINT& size)
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

void RenderPassResource::SetGpuDescriptorHandle(const std::string& name, const std::string& type, CD3DX12_GPU_DESCRIPTOR_HANDLE& handle, const UINT& size)
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

void RenderPassResource::SetResource(const std::string& name, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
{
	resources[name] = resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource>& RenderPassResource::GetResource(const std::string& name)
{
	return resources[name];
}

D3D12_RESOURCE_STATES RenderPassResource::GetResourceState(const std::string& name)
{
	if (states.find(name) != states.end())
	{
		return states[name];
	}
	else
	{
		return D3D12_RESOURCE_STATES();
	}
}

void RenderPassResource::SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state)
{
	if (resources.find(name) == resources.end())
		return;

	if (states.find(name) == states.end())
	{
		if (state != defaultState)
		{
			BarrierTransition::Transition(commandList, GetResource(name).Get(), defaultState, state, name.c_str(), "RenderPassResource");
		}
		states[name] = state;
	}
	else
	{
		if (states[name] != state)
		{
			BarrierTransition::Transition(commandList, GetResource(name).Get(), states[name], state, name.c_str(), "RenderPassResource");
			states[name] = state;
		}
	}
}
