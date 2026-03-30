#include "../EngineApp.h"

void EngineApp::PushRenderItems()
{
	RenderItemSettings lightRenderItems;
	std::unordered_map<std::string, RenderItemSettings> lightRenderSettingsDict;
	lightRenderSettingsDict[mSubsets[GetFullPath("Models/directionallight.obj")][0]->Name] = lightRenderItems;
	lightRenderSettingsDict[mSubsets[GetFullPath("Models/spotlight.obj")][0]->Name] = lightRenderItems;

	UINT dirLightOffset = 0;
	for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++)
	{
		RenderItem::BuildRenderItems(GetFullPath("Models/directionallight.obj"), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
			ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mDirectionalLightRenderItemMap, lightRenderSettingsDict, dirLightOffset);
	}

	UINT spotLightOffset = 0;
	for (int i = 0; i < dynamicLights.SpotLights.size(); i++)
	{
		RenderItem::BuildRenderItems(GetFullPath("Models/spotlight.obj"), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
			ObjectCBIndex, mSubsets, mGeometries, mMaterials, mMesh, mRenderItemLayers["Opaque"], mRenderItems, mSpotLightRenderItemMap, lightRenderSettingsDict, spotLightOffset);
	}

	for (const auto& item : mLevelRenderItems.at("DemoLevel"))
	{
		const std::string& itemName = item.first;
		const ItemData& renderItemData = item.second;
		std::unordered_map<std::string, RenderItemSettings> settings = renderItemData.settings;

		bool geoExists = PathExists(renderItemData.geometry);

		std::string geoPath = renderItemData.geometry;
		if (geoExists)
		{
			geoPath = GetFullPath(renderItemData.geometry.c_str());
		}

		UINT meshOffset = 0;
		if (!renderItemData.animated)
		{

			RenderItem::BuildRenderItems(geoPath, DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
										DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
										DirectX::XMFLOAT3((float)renderItemData.scale[0], (float)renderItemData.scale[1], (float)renderItemData.scale[2]), ObjectCBIndex, mSubsets, mGeometries,
										mMaterials, mMesh, mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap, settings, meshOffset);
		}
		else
		{
			RenderItem::BuildRenderItems(geoPath, renderItemData.animation,
										DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
										DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
										DirectX::XMFLOAT3((float)renderItemData.scale[0], (float)renderItemData.scale[1], (float)renderItemData.scale[2]), ObjectCBIndex, SkinnedCBIndex, BlendCBIndex,
										mSubsets, mGeometries, mMaterials, mMesh, mSkeletons, mAnimations, mTransforms, mSkinningControllers, mBlendshapeControllers, mMeshAnimationResources,
										mRenderItemLayers[renderItemData.render_layer], mRenderItems, mMeshRenderItemMap, md3dDevice, mCommandList, settings, meshOffset);
		}

		// Build MeshInstance with DeformationGraph for this item
		std::string fullPath = geoExists ? GetFullPath(renderItemData.geometry.c_str()) : renderItemData.geometry;
		if (mRenderMeshAssets.count(fullPath) && !mMeshInstances.count(fullPath))
		{
			RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
			DeformationGraph graph;

			if (renderItemData.animated && asset->hasBlendshapes)
			{
				UINT bsCBIdx = (UINT)mBlendshapeDeformers.size();
				auto bsCtrl  = mBlendshapeControllers.count(fullPath)
					? mBlendshapeControllers.at(fullPath) : nullptr;
				auto bs = std::make_unique<BlendshapeDeformer>(
					md3dDevice.Get(), mCommandList.Get(),
					asset, bsCBIdx, bsCtrl,
					mBlendRootSignature.Get(), mPSOs.at("blend").Get());
				mBlendshapeDeformers.push_back(bs.get());
				graph.AddDeformer(std::move(bs));
			}

			if (renderItemData.animated && asset->hasSkinning)
			{
				UINT skinCBIdx = (UINT)mSkinDeformers.size();
				auto skinCtrl  = mSkinningControllers.count(fullPath)
					? mSkinningControllers.at(fullPath) : nullptr;
				auto sk = std::make_unique<SkinDeformer>(
					md3dDevice.Get(), mCommandList.Get(),
					asset, skinCBIdx, skinCtrl,
					mSkinnedRootSignature.Get(), mPSOs.at("skinned").Get());
				mSkinDeformers.push_back(sk.get());
				graph.AddDeformer(std::move(sk));
			}

			if (renderItemData.simulated && mSimMeshAssets.count(fullPath))
			{
				auto phys = std::make_unique<PhysicsDeformer>(
					md3dDevice.Get(), mCommandList.Get(),
					mSimMeshAssets.at(fullPath).get(),
					MakePhysicsDeformerResources());
				graph.AddDeformer(std::move(phys));
			}

			auto instance = std::make_shared<MeshInstance>(asset, std::move(graph));
			mMeshInstances[fullPath] = instance;

			// Assign Instance and SubsetName to each RenderItem for this mesh
			for (auto& layer : mRenderItemLayers)
			{
				for (auto& ri : layer.second)
				{
					if (ri->Geo && ri->Geo->Name == fullPath)
					{
						ri->Instance = instance.get();
						// Find the subset name for this RenderItem by matching DrawArgs
						for (auto& [subsetName, args] : asset->DrawArgs)
						{
							if (args.IndexStart == ri->IndexStart && args.IndexCount == ri->IndexCount)
							{
								ri->SubsetName = subsetName;
								break;
							}
						}
					}
				}
			}
		}
	}
}
