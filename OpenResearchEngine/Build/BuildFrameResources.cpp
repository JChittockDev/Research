#include "../EngineApp.h"

void EngineApp::BuildFrameResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 3, (UINT)mAllRitems.size(), 1, (UINT)mMaterials.size()));
    }
}