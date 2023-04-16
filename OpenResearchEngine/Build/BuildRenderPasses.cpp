#include "../EngineApp.h"

void EngineApp::BuildRenderPasses()
{
    int numLights = dynamicLights.GetNumLights();
    for (int i = 0; i < dynamicLights.GetNumLights(); i++)
    {
        mShadowMaps.push_back(std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048));
    }
    mShadowPassCBs.resize(numLights);
}