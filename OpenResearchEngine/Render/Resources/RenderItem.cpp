#include "RenderItem.h"

DirectX::XMFLOAT4X4 CreateTransformMatrix(const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling)
{
	DirectX::XMVECTOR translationVector = DirectX::XMLoadFloat3(&translation);
	DirectX::XMVECTOR rotationQuaternion = DirectX::XMLoadFloat4(&rotation);
	DirectX::XMVECTOR scalingVector = DirectX::XMLoadFloat3(&scaling);

	DirectX::XMMATRIX scalingMatrix = DirectX::XMMatrixScalingFromVector(scalingVector);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(rotationQuaternion);
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(translationVector);

	DirectX::XMMATRIX transformMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, transformMatrix);
	return matrix;
}

void RenderItem::BuildRenderItems(const std::string& meshName, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling,
									UINT& ObjectCBIndex, UINT& SkinnedCBIndex, const std::unordered_map<std::string, std::vector<Subset>>& subsets, const std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometry,
									const std::unordered_map<std::string, std::unique_ptr<Material>>& materials, const std::vector<ModelMaterial>& modelMaterials, const std::unordered_map<std::string, Mesh>& skinnedMesh, 
									std::vector<RenderItem*>& renderLayers, std::vector<std::unique_ptr<RenderItem>>& renderItems)
{

	DirectX::XMFLOAT4X4& transformMatrix = CreateTransformMatrix(translation, rotation, scaling);

	for (UINT i = 0; i < subsets.at(meshName).size(); ++i)
	{
		std::string submeshName = "sm_" + std::to_string(i);

		auto ritem = std::make_unique<RenderItem>();
		ritem->World = transformMatrix;

		// Reflect to change coordinate system from the RHS the data was exported out as.
		DirectX::XMMATRIX modelScale = DirectX::XMMatrixScaling(0.05f, 0.05f, -0.05f);
		DirectX::XMMATRIX modelRot = DirectX::XMMatrixRotationY(Math::Pi);
		DirectX::XMMATRIX modelOffset = DirectX::XMMatrixTranslation(0.0f, 0.0f, -5.0f);
		XMStoreFloat4x4(&ritem->World, modelScale * modelRot * modelOffset);

		ritem->TexTransform = Math::Identity4x4();
		ritem->ObjCBIndex = ObjectCBIndex++;
		ritem->Geo = geometry.at(meshName).get();

		UINT materialIndex = ritem->Geo->DrawArgs[submeshName].MaterialIndex;
		ritem->Mat = materials.at(modelMaterials.at(materialIndex).Name).get();

		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->BaseVertexLocation = ritem->Geo->DrawArgs[submeshName].BaseVertexLocation;

		ritem->SkinnedCBIndex = SkinnedCBIndex;
		ritem->AnimationInstance = skinnedMesh.at(meshName).mAnimation;

		renderLayers.push_back(ritem.get());
		renderItems.push_back(std::move(ritem));
	}
	SkinnedCBIndex++;
}