#include "../EngineApp.h"

void EngineApp::BuildFrameResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1 + dynamicLights.GetNumLights(), (UINT)mRenderItems.size(), 1, (UINT)mMaterials.size()));
    }
}