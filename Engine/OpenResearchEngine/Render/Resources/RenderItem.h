#pragma once

#include "FrameResource.h"
#include "Mesh.h"
#include "Skinning.h"
#include "Blendshapes.h"
#include "MeshAnimationResource.h"

class RenderItem
{
public:
	RenderItem() {};

	bool Simulation = false;
	
	DirectX::XMFLOAT4X4 World = Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Math::Identity4x4();
	
	int NumFramesDirty = gNumFrameResources;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	UINT VertexCount = 0;
	UINT IndexCount = 0;
	UINT TriangleCount = 0;
	UINT SimMeshVertexCount = 0;
	UINT SimMeshIndexCount = 0;
	UINT SimMeshTriangleCount = 0;
	UINT BlendshapeCount = 0;
	UINT BlendshapeVertexCount = 0;
	UINT SimMeshConstraintCount = 0;

	UINT BlendshapeStart = 0;
	UINT BlendshapeVertexStart = 0;
	UINT SimMeshConstraintStart = 0;

	UINT IndexStart = 0;
	UINT VertexStart = 0;
	UINT TriangleStart = 0;
	UINT SimMeshIndexStart = 0;
	UINT SimMeshVertexStart = 0;
	UINT SimMeshTriangleStart = 0;

	std::vector<BlendshapeSubset> BlendshapeSubsets;

	UINT ObjCBIndex = -1;
	UINT SkinnedCBIndex = -1;
	UINT BlendCBIndex = -1;

	std::shared_ptr<SkinningController> AnimationInstance = nullptr;
	std::shared_ptr<BlendshapeController> BlendshapeInstance = nullptr;
	std::shared_ptr<MeshAnimationResource> MeshAnimationResourceInstance = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	static void BuildRenderItems(const std::string& meshName, const std::string& animationClip, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, UINT& SkinnedCBIndex, UINT& BlendCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
		const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
		const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
		std::unordered_map<std::string, std::shared_ptr<SkinningController>>& SkinningControllers, std::unordered_map<std::string, std::shared_ptr<BlendshapeController>>& BlendshapeControllers, std::unordered_map<std::string, std::shared_ptr<MeshAnimationResource>>& MeshAnimationResources,
		std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, std::unordered_map<std::string, RenderItemSettings>& settings, UINT& offset);

	static void BuildRenderItems(const std::string& meshName, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
		UINT& ObjectCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
		const std::unordered_map<std::string, std::shared_ptr<Material>>& materials,
		const std::unordered_map<std::string, std::shared_ptr<Mesh>>& mesh, std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems,
		std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap, std::unordered_map<std::string, RenderItemSettings>& settings, UINT& offset);
};