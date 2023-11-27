#include "RenderItem.h"

void RenderItem::BuildRenderItems(const std::string& meshName, const std::string& uniqueID, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
	UINT& ObjectCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
	const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector< std::shared_ptr<ModelMaterial>>& modelMaterials,
	const std::unordered_map<std::string, std::shared_ptr<Mesh>>& mesh, std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap)
{
	DirectX::XMFLOAT4X4& transformMatrix = Math::CreateTransformMatrix(translation, rotation, scaling);

	for (UINT i = 0; i < subsets.at(meshName).size(); ++i)
	{
		std::string submeshName = "sm_" + std::to_string(i);
		auto ritem = std::make_shared<RenderItem>();
		ritem->World = transformMatrix;
		ritem->TexTransform = Math::Identity4x4();
		ritem->ObjCBIndex = ObjectCBIndex++;
		ritem->Geo = geometry.at(meshName).get();
		UINT materialIndex = ritem->Geo->DrawArgs[submeshName].MaterialIndex;
		ritem->Mat = materials.at(modelMaterials.at(materialIndex)->Name).get();
		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->VertexCount = ritem->Geo->DrawArgs[submeshName].VertexCount;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->TriangleCount = ritem->Geo->DrawArgs[submeshName].TriangleCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->StartVertexLocation = ritem->Geo->DrawArgs[submeshName].StartVertexLocation;
		ritem->StartTriangleLocation = ritem->Geo->DrawArgs[submeshName].StartTriangleLocation;
		ritem->BlendshapeVertexCount = ritem->Geo->DrawArgs[submeshName].BlendshapeVertexCount;
		ritem->BlendshapeVertexStart = ritem->Geo->DrawArgs[submeshName].BlendshapeVertexStart;
		ritem->BlendshapeSubsets = ritem->Geo->DrawArgs[submeshName].BlendshapeSubsets;
		ritem->SimMeshVertexCount = ritem->Geo->DrawArgs[submeshName].SimMeshVertexCount;
		ritem->SimMeshIndexCount = ritem->Geo->DrawArgs[submeshName].SimMeshIndexCount;
		ritem->SimMeshStretchConstraintCount = ritem->Geo->DrawArgs[submeshName].SimMeshStretchConstraintCount;
		ritem->SimMeshStretchConstraintStart = ritem->Geo->DrawArgs[submeshName].SimMeshStretchConstraintStart;
		ritem->SimMeshBendingConstraintCount = ritem->Geo->DrawArgs[submeshName].SimMeshBendingConstraintCount;
		ritem->SimMeshBendingConstraintStart = ritem->Geo->DrawArgs[submeshName].SimMeshBendingConstraintStart;
		ritem->SimMeshTriangleCount = ritem->Geo->DrawArgs[submeshName].SimMeshTriangleCount;
		ritem->SimMeshStartIndexLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartIndexLocation;
		ritem->SimMeshStartVertexLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartVertexLocation;
		ritem->SimMeshStartTriangleLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartTriangleLocation;
		renderLayers.push_back(ritem);
		
		if (uniqueID.empty())
		{
			renderItemMap[meshName].push_back(ritem);
		}
		else
		{
			renderItemMap[meshName + "_" + uniqueID].push_back(ritem);
		}
		
		renderItems.push_back(std::move(ritem));
	}

}

void RenderItem::BuildRenderItems(const std::string& meshName, const std::string& uniqueID, const std::string& animationClip, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
	UINT& ObjectCBIndex, UINT& SkinnedCBIndex, UINT& BlendCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
	const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector< std::shared_ptr<ModelMaterial>>& modelMaterials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
	const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
	std::unordered_map<std::string, std::shared_ptr<SkinningController>>& SkinningControllers, std::unordered_map<std::string, std::shared_ptr<BlendshapeController>>& BlendshapeControllers, std::unordered_map<std::string, std::shared_ptr<MeshAnimationResource>>& MeshAnimationResources,
	std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList)
{

	DirectX::XMFLOAT4X4& transformMatrix = Math::CreateTransformMatrix(translation, rotation, scaling);
	std::shared_ptr<SkinningController> mSkinningController = std::make_shared<SkinningController>();
	mSkinningController->skeleton = skeletons.at(meshName);
	mSkinningController->rootNode = transforms.at(mSkinningController->skeleton->rootNodeName);
	mSkinningController->animation = animations.at(animationClip);
	mSkinningController->transforms.resize(mSkinningController->skeleton->bones.size());
	mSkinningController->TimePos = 0.0f;
	mSkinningController->Speed = 20.0f;
	mSkinningController->Loop = true;

	std::shared_ptr<BlendshapeController> mBlendshapeController = std::make_shared<BlendshapeController>();
	mBlendshapeController->animation = animations.at(animationClip);
	mBlendshapeController->TimePos = mSkinningController->TimePos;
	mBlendshapeController->Speed = mSkinningController->Speed;
	mBlendshapeController->Loop = mSkinningController->Loop;

	MeshGeometry* geo = geometry.at(meshName).get();
	std::shared_ptr<MeshAnimationResource> mMeshAnimationResource = std::make_shared<MeshAnimationResource>(md3dDevice, mCommandList, geo->DeformationData);
	
	for (UINT i = 0; i < subsets.at(meshName).size(); ++i)
	{

		std::string submeshName = "sm_" + std::to_string(i);
		auto ritem = std::make_shared<RenderItem>();
		ritem->World = transformMatrix;
		ritem->TexTransform = Math::Identity4x4();
		ritem->ObjCBIndex = ObjectCBIndex++;
		ritem->Geo = geometry.at(meshName).get();
		UINT materialIndex = ritem->Geo->DrawArgs[submeshName].MaterialIndex;
		ritem->Mat = materials.at(modelMaterials.at(materialIndex)->Name).get();
		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->VertexCount = ritem->Geo->DrawArgs[submeshName].VertexCount;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->TriangleCount = ritem->Geo->DrawArgs[submeshName].TriangleCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->StartVertexLocation = ritem->Geo->DrawArgs[submeshName].StartVertexLocation;
		ritem->StartTriangleLocation = ritem->Geo->DrawArgs[submeshName].StartTriangleLocation;
		ritem->BlendshapeCount = ritem->Geo->DrawArgs[submeshName].BlendshapeCount;
		ritem->BlendshapeStart = ritem->Geo->DrawArgs[submeshName].BlendshapeStart;
		ritem->BlendshapeVertexCount = ritem->Geo->DrawArgs[submeshName].BlendshapeVertexCount;
		ritem->BlendshapeVertexStart = ritem->Geo->DrawArgs[submeshName].BlendshapeVertexStart;
		ritem->BlendshapeSubsets = ritem->Geo->DrawArgs[submeshName].BlendshapeSubsets;
		ritem->SimMeshVertexCount = ritem->Geo->DrawArgs[submeshName].SimMeshVertexCount;
		ritem->SimMeshIndexCount = ritem->Geo->DrawArgs[submeshName].SimMeshIndexCount;
		ritem->SimMeshStretchConstraintCount = ritem->Geo->DrawArgs[submeshName].SimMeshStretchConstraintCount;
		ritem->SimMeshStretchConstraintStart = ritem->Geo->DrawArgs[submeshName].SimMeshStretchConstraintStart;
		ritem->SimMeshBendingConstraintCount = ritem->Geo->DrawArgs[submeshName].SimMeshBendingConstraintCount;
		ritem->SimMeshBendingConstraintStart = ritem->Geo->DrawArgs[submeshName].SimMeshBendingConstraintStart;
		ritem->SimMeshTriangleCount = ritem->Geo->DrawArgs[submeshName].SimMeshTriangleCount;
		ritem->SimMeshStartIndexLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartIndexLocation;
		ritem->SimMeshStartVertexLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartVertexLocation;
		ritem->SimMeshStartTriangleLocation = ritem->Geo->DrawArgs[submeshName].SimMeshStartTriangleLocation;
		ritem->SkinnedCBIndex = SkinnedCBIndex;
		ritem->BlendCBIndex = BlendCBIndex;
		ritem->AnimationInstance = mSkinningController;
		ritem->MeshAnimationResourceInstance = mMeshAnimationResource;

		mBlendshapeController->weights[subsets.at(meshName).at(i)->alias] = std::vector<float> (ritem->BlendshapeCount, 0.0);


		renderLayers.push_back(ritem);
		
		if (uniqueID.empty())
		{
			renderItemMap[meshName].push_back(ritem);
		}
		else
		{
			renderItemMap[meshName + "_" + uniqueID].push_back(ritem);
		}
		
		renderItems.push_back(std::move(ritem));
	}
	SkinnedCBIndex++;
	BlendCBIndex++;
	SkinningControllers[meshName] = std::move(mSkinningController);
	BlendshapeControllers[meshName] = std::move(mBlendshapeController);
	MeshAnimationResources[meshName] = std::move(mMeshAnimationResource);
}

void RenderItem::BuildRenderItem(const std::string& meshName, const std::string& uniqueID, const std::string& subMeshName, const std::string& materialName, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
	UINT& ObjectCBIndex, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry, const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, std::vector<std::shared_ptr<RenderItem>>& renderLayers,
	std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap)
{
	auto ritem = std::make_shared<RenderItem>();
	DirectX::XMFLOAT4X4& transformMatrix = Math::CreateTransformMatrix(translation, rotation, scaling);
	ritem->World = transformMatrix;
	ritem->TexTransform = Math::Identity4x4();
	ritem->ObjCBIndex = ObjectCBIndex++;
	ritem->Mat = materials.at(materialName).get();
	ritem->Geo = geometry.at(meshName).get();
	ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ritem->VertexCount = ritem->Geo->DrawArgs[subMeshName].VertexCount;
	ritem->IndexCount = ritem->Geo->DrawArgs[subMeshName].IndexCount;
	ritem->TriangleCount = ritem->Geo->DrawArgs[subMeshName].TriangleCount;
	ritem->StartIndexLocation = ritem->Geo->DrawArgs[subMeshName].StartIndexLocation;
	ritem->StartVertexLocation = ritem->Geo->DrawArgs[subMeshName].StartVertexLocation;
	ritem->StartTriangleLocation = ritem->Geo->DrawArgs[subMeshName].StartTriangleLocation;
	ritem->BlendshapeVertexCount = ritem->Geo->DrawArgs[subMeshName].BlendshapeVertexCount;
	ritem->BlendshapeVertexStart = ritem->Geo->DrawArgs[subMeshName].BlendshapeVertexStart;
	ritem->BlendshapeSubsets = ritem->Geo->DrawArgs[subMeshName].BlendshapeSubsets;
	ritem->SimMeshVertexCount = ritem->Geo->DrawArgs[subMeshName].SimMeshVertexCount;
	ritem->SimMeshIndexCount = ritem->Geo->DrawArgs[subMeshName].SimMeshIndexCount;
	ritem->SimMeshStretchConstraintCount = ritem->Geo->DrawArgs[subMeshName].SimMeshStretchConstraintCount;
	ritem->SimMeshStretchConstraintStart = ritem->Geo->DrawArgs[subMeshName].SimMeshStretchConstraintStart;
	ritem->SimMeshBendingConstraintCount = ritem->Geo->DrawArgs[subMeshName].SimMeshBendingConstraintCount;
	ritem->SimMeshBendingConstraintStart = ritem->Geo->DrawArgs[subMeshName].SimMeshBendingConstraintStart;
	ritem->SimMeshTriangleCount = ritem->Geo->DrawArgs[subMeshName].SimMeshTriangleCount;
	ritem->SimMeshStartIndexLocation = ritem->Geo->DrawArgs[subMeshName].SimMeshStartIndexLocation;
	ritem->SimMeshStartVertexLocation = ritem->Geo->DrawArgs[subMeshName].SimMeshStartVertexLocation;
	ritem->SimMeshStartTriangleLocation = ritem->Geo->DrawArgs[subMeshName].SimMeshStartTriangleLocation;
	renderLayers.push_back(ritem);

	if (uniqueID.empty())
	{
		renderItemMap[meshName].push_back(ritem);
	}
	else
	{
		renderItemMap[meshName + "_" + uniqueID].push_back(ritem);
	}

	renderItems.push_back(std::move(ritem));
}