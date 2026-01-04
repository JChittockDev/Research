#include "../EngineApp.h"

void EngineApp::UpdateRadiancePassCB(const GameTimer& gt)
{
    auto radianceCB = mCurrFrameResource->RadianceCB.get();

    int lightIndex = 0;
    for (int i = 0; i < dynamicLights.DirectionalLights.size(); i++, lightIndex++)
    {
        mRadianceCBs[lightIndex].View = mMainPassCB.View;
        mRadianceCBs[lightIndex].InvView = mMainPassCB.InvView;
        mRadianceCBs[lightIndex].Proj = mMainPassCB.Proj;
        mRadianceCBs[lightIndex].InvProj = mMainPassCB.InvProj;
        mRadianceCBs[lightIndex].EyePosW = mMainPassCB.EyePosW;
        mRadianceCBs[lightIndex].LightTransform = mMainPassCB.LightTransforms[lightIndex];
        mRadianceCBs[lightIndex].Light = mMainPassCB.Lights[lightIndex];

        radianceCB->CopyData(lightIndex, mRadianceCBs[lightIndex]);
    }

    for (int i = 0; i < dynamicLights.SpotLights.size(); i++, lightIndex++)
    {
        mRadianceCBs[lightIndex].View = mMainPassCB.View;
        mRadianceCBs[lightIndex].InvView = mMainPassCB.InvView;
        mRadianceCBs[lightIndex].Proj = mMainPassCB.Proj;
        mRadianceCBs[lightIndex].InvProj = mMainPassCB.InvProj;
        mRadianceCBs[lightIndex].EyePosW = mMainPassCB.EyePosW;
        mRadianceCBs[lightIndex].LightTransform = mMainPassCB.LightTransforms[lightIndex];
        mRadianceCBs[lightIndex].Light = mMainPassCB.Lights[lightIndex];

        radianceCB->CopyData(lightIndex, mRadianceCBs[lightIndex]);
    }
}