#include "../EngineApp.h"


void EngineApp::BuildLevel()
{
	LevelReader levelData("E:\\Personal Work\\Research\\OpenResearchEngine\\Levels\\DemoLevel.json");
	
	auto& staticRenderItemDataDict = *levelData.level->data->renderItemData->staticRenderItemDataDict;
	auto& skinnedRenderItemDataDict = *levelData.level->data->renderItemData->skinnedRenderItemDataDict;

	for (const auto& item : staticRenderItemDataDict)
	{
		const std::string& itemName = item.first;
		const StaticRenderItemData& staticRenderItemData = item.second;

		RenderItem::BuildRenderItem(staticRenderItemData.geometry, staticRenderItemData.mesh, staticRenderItemData.material, 
			DirectX::XMFLOAT3(staticRenderItemData.position[0], staticRenderItemData.position[1], staticRenderItemData.position[2]), 
			DirectX::XMFLOAT4(staticRenderItemData.rotation[0], staticRenderItemData.rotation[1], staticRenderItemData.rotation[2], staticRenderItemData.rotation[3]),
			DirectX::XMFLOAT3(staticRenderItemData.scale[0], staticRenderItemData.scale[1], staticRenderItemData.scale[2]),
			ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[staticRenderItemData.render_layer], mAllRitems);
	}

	for (const auto& item : skinnedRenderItemDataDict)
	{
		const std::string& itemName = item.first;
		const SkinnedRenderItemData& staticRenderItemData = item.second;

		RenderItem::BuildRenderItems(staticRenderItemData.geometry, staticRenderItemData.animation,
			DirectX::XMFLOAT3(staticRenderItemData.position[0], staticRenderItemData.position[1], staticRenderItemData.position[2]),
			DirectX::XMFLOAT4(staticRenderItemData.rotation[0], staticRenderItemData.rotation[1], staticRenderItemData.rotation[2], staticRenderItemData.rotation[3]),
			DirectX::XMFLOAT3(staticRenderItemData.scale[0], staticRenderItemData.scale[1], staticRenderItemData.scale[2]),
			ObjectCBIndex, SkinnedCBIndex, mSkinnedSubsets, mGeometries, mMaterials, mSkinnedMats, mSkinnedMesh, mSkeletons, mAnimations, 
			mTransforms, mAnimationControllers, mRitemLayer[staticRenderItemData.render_layer], mAllRitems);
	}
}