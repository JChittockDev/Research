#include "../EngineApp.h"
#include "../Objects/SkinnedMesh.h"
#include "../Objects/StaticMesh.h"

void EngineApp::BuildAssets(std::unordered_map<std::string, std::pair<MeshType, std::string>>& inputData, 
						std::unordered_map<std::string, std::shared_ptr<MeshRenderData>>& meshRenderAssets,
						std::unordered_map<std::string, std::string>& textureDefinitions)
{
	std::unique_ptr<MeshGeometry> genericGeo = GenericGeometry();
	std::unique_ptr<MeshMaterial> genericMats = GenericMaterials();
	textureDefinitions = GenericTextures();
	
	std::unique_ptr<StaticMesh> genericStaticMesh = std::make_unique<StaticMesh>(genericGeo, genericMats);
	std::shared_ptr<MeshRenderData> genericMeshRenderData = std::make_shared<MeshRenderData>();

	genericMeshRenderData->staticMesh = std::move(genericStaticMesh);

	meshRenderAssets["generic"] = genericMeshRenderData;

	// generate mesh types from external sources
	for (auto& [key, val] : inputData)
	{
		std::pair<MeshType, std::string>& meshInput = val;
		MeshType& type = meshInput.first;
		const std::string& filePath = meshInput.second;

		std::shared_ptr<MeshRenderData> meshRenderData = std::make_shared<MeshRenderData>();

		if (type == MeshType::Skinned)
		{
			std::unique_ptr<SkinnedMesh> skinnedMesh = std::make_unique<SkinnedMesh>(md3dDevice, mCommandList, diffuseSrvHeapIndex, normalSrvHeapIndex, matCBIndex, textureDefinitions, filePath);
			meshRenderData->skinnedMesh = std::move(skinnedMesh);
		}
		else
		{
			std::unique_ptr<StaticMesh> staticMesh = std::make_unique<StaticMesh>(md3dDevice, mCommandList, diffuseSrvHeapIndex, normalSrvHeapIndex, matCBIndex, textureDefinitions, filePath);
			meshRenderData->staticMesh = std::move(staticMesh);
		}

		meshRenderAssets[key] = meshRenderData;
	}
}