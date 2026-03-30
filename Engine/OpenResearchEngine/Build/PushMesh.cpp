#include "../EngineApp.h"

void EngineApp::PushMesh()
{
    // ── Light source geometry (static, no animation) ──
    for (const std::string relPath : { "Models/directionallight.obj", "Models/spotlight.obj" }) {
        std::string fullPath = GetFullPath(relPath.c_str());
        if (mRenderMeshAssets.count(fullPath)) continue;
        std::string noVC;
        mRenderMeshAssets[fullPath] = RenderMeshAsset::Load(
            fullPath, noVC, /*forAnimation=*/false,
            md3dDevice, mCommandList, mSubsets);
    }

    // ── Level items ──
    for (const auto& item : mLevelRenderItems.at("DemoLevel"))
    {
        const ItemData& data = item.second;
        if (data.geometry.empty()) continue;
        bool geoExists = PathExists(data.geometry);
        if (!geoExists) continue;

        std::string fullPath = GetFullPath(data.geometry.c_str());
        if (mRenderMeshAssets.count(fullPath)) continue;  // already loaded (shared asset)

        // Vertex colour texture (replaces old .jpg detection)
        std::string vcPath;
        std::string texPath = ChangeExtension(fullPath, ".jpg");
        if (PathExists(texPath)) vcPath = texPath;

        mRenderMeshAssets[fullPath] = RenderMeshAsset::Load(
            fullPath, vcPath, data.animated,
            md3dDevice, mCommandList, mSubsets);

        if (data.simulated) {
            mSimMeshAssets[fullPath] = SimMeshAsset::Load(
                fullPath, vcPath,
                md3dDevice, mCommandList,
                mRenderMeshAssets[fullPath]->DrawArgs);
        }
    }
}
