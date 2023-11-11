#include "../EngineApp.h"

void EngineApp::SetFenceResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_shared<FrameResource>(md3dDevice.Get(), 1 + dynamicLights.GetNumLights(), (UINT)mRenderItems.size(), 1, 1, (UINT)mMaterials.size()));
    }
}