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
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->BaseVertexLocation = ritem->Geo->DrawArgs[submeshName].BaseVertexLocation;
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
	UINT& ObjectCBIndex, UINT& SkinnedCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
	const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::vector< std::shared_ptr<ModelMaterial>>& modelMaterials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
	const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
	std::unordered_map<std::string, std::shared_ptr<AnimationController>>& animationControllers, std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap)
{

	DirectX::XMFLOAT4X4& transformMatrix = Math::CreateTransformMatrix(translation, rotation, scaling);
	std::shared_ptr<AnimationController> mAnimationController = std::make_shared<AnimationController>();
	mAnimationController->skeleton = skeletons.at(meshName);
	mAnimationController->rooNode = transforms.at(mAnimationController->skeleton->rootNodeName);
	mAnimationController->animation = animations.at(animationClip);
	mAnimationController->transforms.resize(mAnimationController->skeleton->bones.size());
	mAnimationController->TimePos = 0.0f;
	mAnimationController->Speed = 20.0f;
	mAnimationController->Loop = true;
	
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
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->BaseVertexLocation = ritem->Geo->DrawArgs[submeshName].BaseVertexLocation;
		ritem->SkinnedCBIndex = SkinnedCBIndex;
		ritem->AnimationInstance = mAnimationController;
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
	animationControllers[meshName] = std::move(mAnimationController);
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
	ritem->IndexCount = ritem->Geo->DrawArgs[subMeshName].IndexCount;
	ritem->StartIndexLocation = ritem->Geo->DrawArgs[subMeshName].StartIndexLocation;
	ritem->BaseVertexLocation = ritem->Geo->DrawArgs[subMeshName].BaseVertexLocation;
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
