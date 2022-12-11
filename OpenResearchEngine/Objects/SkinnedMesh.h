#pragma once
#include "Object.h"
#include "../Render/Resources/SkinnedData.h"
#include "../Common/Structures.h"

class SkinnedMesh : public Object
{
public:
	SkinnedMesh();
	
	SkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		UINT& diffuseSrvHeapIndex, UINT& normalSrvHeapIndex, UINT& matCBIndex, 
		std::unordered_map<std::string, std::string>& textureDefinitions, const std::string& filePath);
	
	SkinnedMesh(std::unique_ptr<MeshGeometry>& geometry, std::unique_ptr<MeshMaterial>& materials,
			SkinnedData* skinningData, std::vector<DirectX::XMFLOAT4X4>& boneTransforms);

public:
	void UpdateTransforms(const std::string& animClip, float& clipTime);

	void LoadSkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		UINT& diffuseSrvHeapIndex, UINT& normalSrvHeapIndex, UINT& matCBIndex, 
		std::unordered_map<std::string, std::string>& textureDefinitions, const std::string& filePath);

public:
	UINT objCBIndex = -1;
	UINT skinnedCBIndex = -1;

	SkinnedData* skinningData = nullptr;
	std::vector<DirectX::XMFLOAT4X4> boneTransforms;

	std::unique_ptr<MeshGeometry> geometry;
	std::unique_ptr<MeshMaterial> materials;

	std::string activeMaterial;

	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	int baseVertexLocation = 0;

	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	XMFLOAT4X4 textureTransform = Math::Identity4x4();
};