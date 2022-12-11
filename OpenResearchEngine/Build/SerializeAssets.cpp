#include "../EngineApp.h"

void EngineApp::SerializeAssets()
{
	// Soldier Mesh
	const std::string skinnedModelFilename = "Models\\soldier.m3d";
	MeshType type = MeshType::Skinned;

	std::pair<MeshType, std::string> inputPair = std::make_pair(type, skinnedModelFilename);

	std::string name = "solider";

	meshDefinitions[name] = inputPair;

}