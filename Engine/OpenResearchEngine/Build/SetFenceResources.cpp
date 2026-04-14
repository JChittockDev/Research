#include "../EngineApp.h"

void EngineApp::SetFenceResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_shared<RenderPassConstantBuffers>(
            md3dDevice.Get(),
            dynamicLights.GetNumLights(),
            (UINT)mRenderItems.size(),
            1, 1,
            (UINT)mAssets->mMaterials.size()));
    }
}
