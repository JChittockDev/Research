#include "../EngineApp.h"

void EngineApp::CreateRtvAndDsvDescriptorHeaps()
{
	// Add +1 for screen normal map, +2 for ambient maps.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 3;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	// Add +1 DSV for shadow map.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1 + MaxLights;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void EngineApp::PopulateDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 64;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	std::vector<ComPtr<ID3D12Resource>> tex2DList =
	{
		mTextures["bricksDiffuseMap"]->Resource,
		mTextures["bricksNormalMap"]->Resource,
		mTextures["tileDiffuseMap"]->Resource,
		mTextures["tileNormalMap"]->Resource,
		mTextures["defaultDiffuseMap"]->Resource,
		mTextures["defaultNormalMap"]->Resource
	};

	mLayoutIndicies["mSkinnedSrvHeapStart"] = std::make_pair((UINT)tex2DList.size(), mCbvSrvUavDescriptorSize);

	for (UINT i = 0; i < (UINT)mTextureNames.size(); ++i)
	{
		auto texResource = mTextures[mTextureNames[i]]->Resource;
		assert(texResource != nullptr);
		tex2DList.push_back(texResource);
	}

	auto skyCubeMap = mTextures["skyCubeMap"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (UINT i = 0; i < (UINT)tex2DList.size(); ++i)
	{
		srvDesc.Format = tex2DList[i]->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
		md3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	}

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyCubeMap->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyCubeMap->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(skyCubeMap.Get(), &srvDesc, hDescriptor);

	mLayoutIndicies["mSkyTexHeapIndex"] = std::make_pair((UINT)tex2DList.size(), mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mShadowMapHeapIndex"] = std::make_pair(mLayoutIndicies["mSkyTexHeapIndex"].first + 1, mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mSsaoHeapIndexStart"] = std::make_pair(mLayoutIndicies["mShadowMapHeapIndex"].first + dynamicLights.GetNumLights(), mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mSsaoAmbientMapIndex"] = std::make_pair(mLayoutIndicies["mSsaoHeapIndexStart"].first + 3, mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mNullCubeSrvIndex"] = std::make_pair(mLayoutIndicies["mSsaoAmbientMapIndex"].first + 5, mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mNullTexSrvIndex1"] = std::make_pair(mLayoutIndicies["mNullCubeSrvIndex"].first + 1, mCbvSrvUavDescriptorSize);
	mLayoutIndicies["mNullTexSrvIndex2"] = std::make_pair(mLayoutIndicies["mNullTexSrvIndex1"].first + 1, mCbvSrvUavDescriptorSize);

	auto nullSrv = GetCpuSrv(mLayoutIndicies["mNullCubeSrvIndex"].first);
	mNullSrv = GetGpuSrv(mLayoutIndicies["mNullCubeSrvIndex"].first);

	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);
	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
}