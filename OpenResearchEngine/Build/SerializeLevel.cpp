#include "../EngineApp.h"

void EngineApp::SerializeLevel()
{
	LevelReader levelData("D:/Programming/Research/OpenResearchEngine/Levels/DemoLevel.json");
	mLevelRenderItems["DemoLevel"] = *levelData.level->data->renderItemData->renderItemDataDict;
	mLevelMaterials["DemoLevel"] = *levelData.level->materialData->pbrMaterialDataDict;
	mLevelLights["DemoLevel"] = *levelData.level->lightData->lightDataDict;
}