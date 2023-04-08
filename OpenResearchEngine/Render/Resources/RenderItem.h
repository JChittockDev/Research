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

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	UINT ObjCBIndex = -1;
	UINT SkinnedCBIndex = -1;

	std::shared_ptr<AnimationController> AnimationInstance = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	static void BuildRenderItems(const std::string& meshName, const std::string& animationClip, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, UINT& SkinnedCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
		const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector< std::shared_ptr<ModelMaterial>>& modelMaterials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
		const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
		std::unordered_map<std::string, std::shared_ptr<AnimationController>>& animationControllers, std::vector<RenderItem*>& renderLayers, std::vector<std::unique_ptr<RenderItem>>& renderItems);

	static void BuildRenderItem(const std::string& meshName, const std::string& subMeshName, const std::string& materialName, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry, const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
		std::vector<RenderItem*>& renderLayers, std::vector<std::unique_ptr<RenderItem>>& renderItems);
};