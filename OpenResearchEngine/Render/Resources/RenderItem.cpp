#include "RenderItem.h"

void RenderItem::BuildRenderItems(const std::string& meshName, const std::string& uniqueID, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
	UINT& ObjectCBIndex, const std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometry,
	const std::unordered_map<std::string, std::shared_ptr<Material>>& materials,
	const std::unordered_map<std::string, std::shared_ptr<Mesh>>& mesh, std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, 
	std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap, std::unordered_map<std::string, RenderItemSettings>& settings)
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
		ritem->Mat = materials.at(settings[subsets.at(meshName).at(i)->alias].Material).get();
		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->VertexCount = ritem->Geo->DrawArgs[submeshName].VertexCount;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->TriangleCount = ritem->Geo->DrawArgs[submeshName].TriangleCount;
		ritem->IndexStart = ritem->Geo->DrawArgs[submeshName].IndexStart;
		ritem->VertexStart = ritem->Geo->DrawArgs[submeshName].VertexStart;
		ritem->TriangleStart = ritem->Geo->DrawArgs[submeshName].TriangleStart;
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
		ritem->SimMeshIndexStart = ritem->Geo->DrawArgs[submeshName].SimMeshIndexStart;
		ritem->SimMeshVertexStart = ritem->Geo->DrawArgs[submeshName].SimMeshVertexStart;
		ritem->SimMeshTriangleStart = ritem->Geo->DrawArgs[submeshName].SimMeshTriangleStart;
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
	const std::unordered_map<std::string, std::shared_ptr<Material>>& materials, const std::unordered_map<std::string, std::shared_ptr<Mesh>>& skinnedMesh,
	const std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons, const std::unordered_map<std::string, std::shared_ptr<Animation>>& animations, const std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms,
	std::unordered_map<std::string, std::shared_ptr<SkinningController>>& SkinningControllers, std::unordered_map<std::string, std::shared_ptr<BlendshapeController>>& BlendshapeControllers, std::unordered_map<std::string, std::shared_ptr<MeshAnimationResource>>& MeshAnimationResources,
	std::vector<std::shared_ptr<RenderItem>>& renderLayers, std::vector<std::shared_ptr<RenderItem>>& renderItems, std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& renderItemMap, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, std::unordered_map<std::string, RenderItemSettings>& settings)
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
		ritem->Mat = materials.at(settings[subsets.at(meshName).at(i)->alias].Material).get();
		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->VertexCount = ritem->Geo->DrawArgs[submeshName].VertexCount;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->TriangleCount = ritem->Geo->DrawArgs[submeshName].TriangleCount;
		ritem->IndexStart = ritem->Geo->DrawArgs[submeshName].IndexStart;
		ritem->VertexStart = ritem->Geo->DrawArgs[submeshName].VertexStart;
		ritem->TriangleStart = ritem->Geo->DrawArgs[submeshName].TriangleStart;
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
		ritem->SimMeshIndexStart = ritem->Geo->DrawArgs[submeshName].SimMeshIndexStart;
		ritem->SimMeshVertexStart = ritem->Geo->DrawArgs[submeshName].SimMeshVertexStart;
		ritem->SimMeshTriangleStart = ritem->Geo->DrawArgs[submeshName].SimMeshTriangleStart;
		ritem->SkinnedCBIndex = SkinnedCBIndex;
		ritem->BlendCBIndex = BlendCBIndex;
		ritem->AnimationInstance = mSkinningController;
		ritem->MeshAnimationResourceInstance = mMeshAnimationResource;
		ritem->Simulation = settings[subsets.at(meshName).at(i)->alias].Simulation;

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