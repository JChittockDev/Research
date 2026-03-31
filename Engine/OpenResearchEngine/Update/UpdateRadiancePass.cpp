#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Render/Resources/FrameResource.h"
#include "../Assets/AssetManager.h"
#include "../Utilities/GameTimer.h"

void UpdateRadiancePassCB(const GameTimer& gt, SceneState& state, AssetManager& assets, FrameResource* fr)
{
    auto radianceCB = fr->RadianceCB.get();

    int lightIndex = 0;
    for (int i = 0; i < (int)state.lights.DirectionalLights.size(); i++, lightIndex++)
    {
        state.radianceCBs[lightIndex].View = state.mainPassCB.View;
        state.radianceCBs[lightIndex].InvView = state.mainPassCB.InvView;
        state.radianceCBs[lightIndex].Proj = state.mainPassCB.Proj;
        state.radianceCBs[lightIndex].InvProj = state.mainPassCB.InvProj;
        state.radianceCBs[lightIndex].EyePosW = state.mainPassCB.EyePosW;
        state.radianceCBs[lightIndex].LightTransform = state.mainPassCB.LightTransforms[lightIndex];
        state.radianceCBs[lightIndex].Light = state.mainPassCB.Lights[lightIndex];

        radianceCB->CopyData(lightIndex, state.radianceCBs[lightIndex]);
    }

    for (int i = 0; i < (int)state.lights.SpotLights.size(); i++, lightIndex++)
    {
        state.radianceCBs[lightIndex].View = state.mainPassCB.View;
        state.radianceCBs[lightIndex].InvView = state.mainPassCB.InvView;
        state.radianceCBs[lightIndex].Proj = state.mainPassCB.Proj;
        state.radianceCBs[lightIndex].InvProj = state.mainPassCB.InvProj;
        state.radianceCBs[lightIndex].EyePosW = state.mainPassCB.EyePosW;
        state.radianceCBs[lightIndex].LightTransform = state.mainPassCB.LightTransforms[lightIndex];
        state.radianceCBs[lightIndex].Light = state.mainPassCB.Lights[lightIndex];

        radianceCB->CopyData(lightIndex, state.radianceCBs[lightIndex]);
    }
}
