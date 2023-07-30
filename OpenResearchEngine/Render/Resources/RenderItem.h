#pragma once

#include "FrameResource.h"
#include "Mesh.h"
#include "Animation.h"

class RenderItem
{
public:
	RenderItem() {};
	
	DirectX::XMFLOAT4X4 World = Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Math::Identity4x4();
	
	int NumFramesDirty = gNumFrameResources;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	UINT VertexCount = 0;
	UINT IndexCount = 0;
	UINT TriangleCount = 0;
	UINT StartIndexLocation = 0;
	UINT StartVertexLocation = 0;
	UINT StartTriangleLocation = 0;

	UINT SimMeshVertexCount = 0;
	UINT SimMeshIndexCount = 0;
	UINT SimMeshEdgeCount = 0;
	UINT SimMeshTriangleCount = 0;
	UINT SimMeshStartIndexLocation = 0;
	UINT SimMeshStartVertexLocation = 0;
	UINT SimMeshStartEdgeLocation = 0;
	UINT SimMeshStartTriangleLocation = 0;

	UINT ObjCBIndex = -1;
	UINT SkinnedCBIndex = -1;

	std::shared_ptr<AnimationController> AnimationInstance = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	static void BuildRenderItems(const std::string& meshName, const std::string& uniqueID, const std::string& animationClip, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, UINT& SkinnedCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
		const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector<std::shared_ptr<ModelMaterial>>& modelMaterials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
		const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
		std::unordered_map<std::string, std::shared_ptr<AnimationController>>& animationControllers, std::vector<std::shared_ptr<RenderItem>>& renderLayers, 
		std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap);

	static void BuildRenderItems(const std::string& meshName, const std::string& uniqueID, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
		const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector< std::shared_ptr<ModelMaterial>>& modelMaterials,
		const std::unordered_map<std::string, std::shared_ptr<Mesh>>& mesh, std::vector<std::shared_ptr<RenderItem>>& renderLayers, 
		std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap);

	static void BuildRenderItem(const std::string& meshName, const std::string& uniqueID, const std::string& subMeshName, const std::string& materialName, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry, const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, std::vector<std::shared_ptr<RenderItem>>& renderLayers,
		std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap);
};