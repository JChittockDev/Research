#pragma once
#include "Object.h"
#include "../Render/Resources/SkinnedData.h"
#include "../Common/Structures.h"

class SkinnedMesh : public Object
{
public:
	SkinnedMesh();
	SkinnedMesh(std::shared_ptr<MeshGeometry>& geometry, std::vector<std::shared_ptr<Material>>& materials,
				SkinnedData* skinningData, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) 
				{_geometry = geometry; _materials = materials, _skinningData = skinningData, 
				_boneTransforms = boneTransforms;};

public:
	void LoadSkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device>& device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		UINT& srvHeapIndex, UINT& matCBIndex, const std::string& filePath);

public:
	UINT _objCBIndex = -1;
	UINT _matCBIndex = -1;
	UINT _srvHeapIndex = -1;
	UINT _skinnedCBIndex = -1;

	int _numFramesDirty = 0;

	SkinnedData* _skinningData = nullptr;
	std::vector<DirectX::XMFLOAT4X4> _boneTransforms;

	std::shared_ptr<MeshGeometry> _geometry;
	std::vector<std::shared_ptr<Material>> _materials;

	UINT _indexCount = 0;
	UINT _startIndexLocation = 0;
	int _baseVertexLocation = 0;

	D3D12_PRIMITIVE_TOPOLOGY _primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	XMFLOAT4X4 _textureTransform = Math::Identity4x4();
};