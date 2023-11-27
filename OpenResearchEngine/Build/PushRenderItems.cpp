#include "../EngineApp.h"

void EngineApp::PushRenderItems()
{
	for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\directionallight.obj", std::to_string(i), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex,
			mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mLightRenderItemMap);
	}

	for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
	{
		RenderItem::BuildRenderItems("Models\\spotlight.obj", std::to_string(i), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex,
			mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mLightRenderItemMap);
	}

	for (const auto& item : mLevelRenderItems.at("DemoLevel"))
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
					ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMats, mMesh, mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap);
			}
			else
			{
				RenderItem::BuildRenderItem(renderItemData.geometry, "", renderItemData.mesh, renderItemData.material,
					DirectX::XMFLOAT3(renderItemData.position[0], renderItemData.position[1], renderItemData.position[2]),
					DirectX::XMFLOAT4(renderItemData.rotation[0], renderItemData.rotation[1], renderItemData.rotation[2], renderItemData.rotation[3]),
					DirectX::XMFLOAT3(renderItemData.scale[0], renderItemData.scale[1], renderItemData.scale[2]),
					ObjectCBIndex, mGeometries, mMaterials, mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap);
			}
		}
		else
		{
			RenderItem::BuildRenderItems(renderItemData.geometry, "", renderItemData.animation,
				DirectX::XMFLOAT3(renderItemData.position[0], renderItemData.position[1], renderItemData.position[2]),
				DirectX::XMFLOAT4(renderItemData.rotation[0], renderItemData.rotation[1], renderItemData.rotation[2], renderItemData.rotation[3]),
				DirectX::XMFLOAT3(renderItemData.scale[0], renderItemData.scale[1], renderItemData.scale[2]),
				ObjectCBIndex, SkinnedCBIndex, BlendCBIndex, mSubsets, mGeometries, mMaterials, mMats, mMesh, mSkeletons, mAnimations,
				mTransforms, mSkinningControllers, mBlendshapeControllers, mMeshAnimationResources, mRenderItemLayers[renderItemData.render_layer],
				mRenderItems, mMeshRenderItemMap, md3dDevice, mCommandList);
		}
	}
}