#include "../EngineApp.h"
#include "../Objects/SkinnedMesh.h"
#include "../Objects/StaticMesh.h"

void EngineApp::BuildMesh(std::unordered_map<std::string, std::pair<MeshType, const std::string>>& inputData)
{

	std::shared_ptr<MeshGeometry> genericGeo = GenericGeometry();
	std::vector<std::shared_ptr<Material>> genericMats = GenericMaterials();

	StaticMesh staticMesh(genericGeo, genericMats);

	// generate mesh types from external sources
	for (auto& [key, val] : inputData)
	{
		std::pair<MeshType, const std::string>& meshInput = val;
		MeshType& type = meshInput.first;
		const std::string& filePath = meshInput.second;

		if (type == MeshType::Skinned)
		{
			SkinnedMesh skinnedMesh(md3dDevice, mCommandList, srvHeapIndex, matCBIndex, filePath);
		}
		else
		{
			StaticMesh staticMesh(md3dDevice, mCommandList, srvHeapIndex, matCBIndex, filePath);
		}
	}
}