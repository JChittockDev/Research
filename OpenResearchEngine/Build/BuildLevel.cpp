#include "../EngineApp.h"


void EngineApp::BuildLevel()
{
	LevelReader levelData("E:\\Personal Work\\Research\\OpenResearchEngine\\Levels\\DemoLevel.json");
	auto& renderItemDataDict = *levelData.level->data->renderItemData->renderItemDataDict;

	for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\directionallight.obj", std::to_string(i), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex,
			mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayer[(int)RenderLayer::Opaque], mRenderItems, mLightRenderItemMap);
	}

	for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\spotlight.obj", std::to_string(i), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex,
			mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayer[(int)RenderLayer::Opaque], mRenderItems, mLightRenderItemMap);
	}

	for (const auto& item : renderItemDataDict)
	{
		const std::string& itemName = item.first;
		const ItemData& renderItemData = item.second;

		if (renderItemData.animation.empty())
		{
			if (renderItemData.mesh.empty())
			{
				RenderItem::BuildRenderItems(renderItemData.geometry, "", DirectX::XMFLOAT3(renderItemData.position[0], renderItemData.position[1], renderItemData.position[2]),
					DirectX::XMFLOAT4(renderItemData.rotation[0], renderItemData.rotation[1], renderItemData.rotation[2], renderItemData.rotation[3]),
					DirectX::XMFLOAT3(renderItemData.scale[0], renderItemData.scale[1], renderItemData.scale[2]),
					ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayer[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap);
			}
			else
			{
				RenderItem::BuildRenderItem(renderItemData.geometry, "", renderItemData.mesh, renderItemData.material,
					DirectX::XMFLOAT3(renderItemData.position[0], renderItemData.position[1], renderItemData.position[2]),
					DirectX::XMFLOAT4(renderItemData.rotation[0], renderItemData.rotation[1], renderItemData.rotation[2], renderItemData.rotation[3]),
					DirectX::XMFLOAT3(renderItemData.scale[0], renderItemData.scale[1], renderItemData.scale[2]),
					ObjectCBIndex, mGeometries, mMaterials, mRenderItemLayer[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap);
			}
		}
		else
		{
			RenderItem::BuildRenderItems(renderItemData.geometry, "", renderItemData.animation,
				DirectX::XMFLOAT3(renderItemData.position[0], renderItemData.position[1], renderItemData.position[2]),
				DirectX::XMFLOAT4(renderItemData.rotation[0], renderItemData.rotation[1], renderItemData.rotation[2], renderItemData.rotation[3]),
				DirectX::XMFLOAT3(renderItemData.scale[0], renderItemData.scale[1], renderItemData.scale[2]),
				ObjectCBIndex, SkinnedCBIndex, mSubsets, mGeometries, mMaterials, mMats, mMesh, mSkeletons, mAnimations,
				mTransforms, mAnimationControllers, mRenderItemLayer[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap);
		}
	}
}