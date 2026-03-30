#include "../EngineApp.h"

void EngineApp::PushRenderItems()
{
    // ── Helper lambda: create a MeshInstance (with optional deformers) and assign
    //    to all RenderItems in the given map whose SubsetName lives in the asset.
    auto BuildAndAssign = [&](
        const std::string&  fullPath,
        const ItemData*     data,        // nullptr for light meshes
        RenderMeshAsset*    asset,
        std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& riMap)
    {
        if (mMeshInstances.count(fullPath)) return;   // already built

        DeformationGraph graph;

        if (data && data->animated && asset->hasBlendshapes)
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

        if (data && data->animated && asset->hasSkinning)
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

        if (data && data->simulated && mSimMeshAssets.count(fullPath))
        {
            auto phys = std::make_unique<PhysicsDeformer>(
                md3dDevice.Get(), mCommandList.Get(),
                mSimMeshAssets.at(fullPath).get(),
                MakePhysicsDeformerResources());
            graph.AddDeformer(std::move(phys));
        }

        auto instance = std::make_shared<MeshInstance>(asset, std::move(graph));
        mMeshInstances[fullPath] = instance;

        // Assign Instance + SubsetName to every RenderItem in the given layer map
        for (auto& [key, vec] : riMap)
        {
            for (auto& ri : vec)
            {
                // Match by ObjCBIndex is not reliable here; match by checking
                // whether the subset name exists in this asset's DrawArgs.
                if (ri->Instance) continue;  // already assigned
                if (asset->DrawArgs.count(ri->SubsetName))
                {
                    ri->Instance = instance.get();
                }
            }
        }
    };

    // ── Light source render items ──
    // Directional lights
    {
        const std::string relPath = "Models/directionallight.obj";
        const std::string fullPath = GetFullPath(relPath.c_str());
        RenderItemSettings settings;
        std::unordered_map<std::string, RenderItemSettings> settingsDict;

        if (mRenderMeshAssets.count(fullPath))
        {
            RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
            UINT lightOffset = 0;
            for (int i = 0; i < (int)dynamicLights.DirectionalLights.size(); i++)
            {
                for (auto& [subsetName, args] : asset->DrawArgs)
                {
                    auto ritem = std::make_shared<RenderItem>();
                    ritem->World       = Math::Identity4x4();
                    ritem->TexTransform = Math::Identity4x4();
                    ritem->ObjCBIndex  = ObjectCBIndex++;
                    ritem->Mat         = mMaterials.at("default").get();
                    ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    ritem->SubsetName  = subsetName;

                    mRenderItemLayers["Opaque"].push_back(ritem);
                    mRenderItems.push_back(ritem);
                    std::string key = subsetName + "_" + std::to_string(lightOffset++);
                    mDirectionalLightRenderItemMap[key].push_back(ritem);
                }
            }

            BuildAndAssign(fullPath, nullptr, asset, mDirectionalLightRenderItemMap);
            // Also assign instance to items in Opaque layer
            for (auto& ri : mRenderItemLayers["Opaque"])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = mMeshInstances.at(fullPath).get();
            }
        }
    }

    // Spot lights
    {
        const std::string relPath = "Models/spotlight.obj";
        const std::string fullPath = GetFullPath(relPath.c_str());

        if (mRenderMeshAssets.count(fullPath))
        {
            RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
            UINT lightOffset = 0;
            for (int i = 0; i < (int)dynamicLights.SpotLights.size(); i++)
            {
                for (auto& [subsetName, args] : asset->DrawArgs)
                {
                    auto ritem = std::make_shared<RenderItem>();
                    ritem->World       = Math::Identity4x4();
                    ritem->TexTransform = Math::Identity4x4();
                    ritem->ObjCBIndex  = ObjectCBIndex++;
                    ritem->Mat         = mMaterials.at("default").get();
                    ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    ritem->SubsetName  = subsetName;

                    mRenderItemLayers["Opaque"].push_back(ritem);
                    mRenderItems.push_back(ritem);
                    std::string key = subsetName + "_" + std::to_string(lightOffset++);
                    mSpotLightRenderItemMap[key].push_back(ritem);
                }
            }

            BuildAndAssign(fullPath, nullptr, asset, mSpotLightRenderItemMap);
            for (auto& ri : mRenderItemLayers["Opaque"])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = mMeshInstances.at(fullPath).get();
            }
        }
    }

    // ── Level render items ──
    for (const auto& item : mLevelRenderItems.at("DemoLevel"))
    {
        const std::string& itemName = item.first;
        const ItemData& renderItemData = item.second;

        if (renderItemData.geometry.empty()) continue;
        bool geoExists = PathExists(renderItemData.geometry);
        if (!geoExists) continue;

        std::string fullPath = GetFullPath(renderItemData.geometry.c_str());
        if (!mRenderMeshAssets.count(fullPath)) continue;

        RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(
            DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
            DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
            DirectX::XMFLOAT3((float)renderItemData.scale[0],    (float)renderItemData.scale[1],    (float)renderItemData.scale[2]));

        const auto& settings = renderItemData.settings;

        for (auto& [subsetName, args] : asset->DrawArgs)
        {
            // Only add a RenderItem if there's a settings entry for this subset
            if (settings.find(subsetName) == settings.end()) continue;

            auto ritem = std::make_shared<RenderItem>();
            ritem->World        = transformMatrix;
            ritem->TexTransform = Math::Identity4x4();
            ritem->ObjCBIndex   = ObjectCBIndex++;
            ritem->Mat          = mMaterials.at(settings.at(subsetName).Material).get();
            ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            ritem->SubsetName   = subsetName;

            mRenderItemLayers[renderItemData.render_layer].push_back(ritem);
            mRenderItems.push_back(ritem);
            mMeshRenderItemMap[itemName].push_back(ritem);
        }

        // Build MeshInstance with DeformationGraph (once per fullPath)
        BuildAndAssign(fullPath, &renderItemData, asset, mMeshRenderItemMap);

        // Assign instance to the items we just pushed into the render layer
        if (mMeshInstances.count(fullPath))
        {
            auto& inst = mMeshInstances.at(fullPath);
            for (auto& ri : mRenderItemLayers[renderItemData.render_layer])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = inst.get();
            }
        }
    }
}
