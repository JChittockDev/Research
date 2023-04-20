#include "../EngineApp.h"

void EngineApp::SerializeLevel()
{
	LevelReader levelData("E:\\Personal Work\\Research\\OpenResearchEngine\\Levels\\DemoLevel.json");
	mLevelRenderItems["DemoLevel"] = *levelData.level->data->renderItemData->renderItemDataDict;
}