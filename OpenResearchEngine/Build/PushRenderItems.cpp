#include "../EngineApp.h"

void EngineApp::PushRenderItems()
{
	RenderItemSettings lightRenderItems;
	std::unordered_map<std::string, RenderItemSettings> lightRenderSettingsDict;
	lightRenderSettingsDict[mSubsets["Models\\directionallight.obj"][0]->Name] = lightRenderItems;
	lightRenderSettingsDict[mSubsets["Models\\spotlight.obj"][0]->Name] = lightRenderItems;

	UINT dirLightOffset = 0;
	for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\directionallight.obj", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), 
			ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mDirectionalLightRenderItemMap, lightRenderSettingsDict, dirLightOffset);
	}

	UINT spotLightOffset = 0;
	for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\spotlight.obj", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), 
			ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mSpotLightRenderItemMap, lightRenderSettingsDict, spotLightOffset);
	}

	for (const auto& item : mLevelRenderItems.at("DemoLevel"))
	{
		const std::string& itemName = item.first;
		const ItemData& renderItemData = item.second;
		std::unordered_map<std::string, RenderItemSettings> settings = renderItemData.settings;
		
		UINT meshOffset = 0;
		if (!renderItemData.deformable)
		{

			RenderItem::BuildRenderItems(renderItemData.geometry, DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
										DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
										DirectX::XMFLOAT3((float)renderItemData.scale[0], (float)renderItemData.scale[1], (float)renderItemData.scale[2]), ObjectCBIndex, mSubsets, mGeometries, 
										mMaterials, mMesh, mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap, settings, meshOffset);
		}
		else
		{
			RenderItem::BuildRenderItems(renderItemData.geometry, renderItemData.animation,
										DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
										DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
										DirectX::XMFLOAT3((float)renderItemData.scale[0], (float)renderItemData.scale[1], (float)renderItemData.scale[2]), ObjectCBIndex, SkinnedCBIndex, BlendCBIndex,
										mSubsets, mGeometries, mMaterials, mMesh, mSkeletons, mAnimations, mTransforms, mSkinningControllers, mBlendshapeControllers, mMeshAnimationResources, 
										mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap, md3dDevice, mCommandList, settings, meshOffset);
		}
	}
}