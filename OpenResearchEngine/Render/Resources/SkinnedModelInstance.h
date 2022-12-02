#pragma once

#include "SkinnedData.h"

struct SkinnedModelInstance
{
    SkinnedData* SkinnedInfo = nullptr;
    std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
    std::string ClipName;
    float TimePos = 0.0f;

    // Called every frame and increments the time position, interpolates the 
    // animations for each bone based on the current animation clip, and 
    // generates the final transforms which are ultimately set to the effect
    // for processing in the vertex shader.
    void UpdateSkinnedAnimation(float dt)
    {
        TimePos += dt;

        // Loop animation
        if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
            TimePos = 0.0f;

        // Compute the final transforms for this time position.
        SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
    }
};