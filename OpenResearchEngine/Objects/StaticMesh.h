#pragma once

#include "Object.h"
#include "../Common/Structures.h"

class StaticMesh : public Object
{
public:
	StaticMesh();

	StaticMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		UINT& srvHeapIndex, UINT& matCBIndex, const std::string& filePath);
	
	StaticMesh(std::unique_ptr<MeshGeometry>& geometry,
		std::unique_ptr<MeshMaterial>& materials);

public:
	void LoadStaticMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		UINT& srvHeapIndex, UINT& matCBIndex, const std::string& filePath);

public:
	UINT objCBIndex = -1;
	UINT matCBIndex = -1;
	UINT srvHeapIndex = -1;

	std::unique_ptr<MeshGeometry> geometry;
	std::unique_ptr<MeshMaterial> materials;

	std::string activeMaterial;

	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	int baseVertexLocation = 0;

	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	XMFLOAT4X4 textureTransform = Math::Identity4x4();
};
