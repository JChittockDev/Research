#pragma once

#include "Skeleton.h"

struct Animation
{
    Skeleton* skeleton;
    aiNode* rootNode;
    aiAnimation* animation;
    std::vector<DirectX::XMFLOAT4X4> transforms;
    float TimePos = 0.0f;
    bool Loop = true;
    float Speed = 1.0f;

    void UpdateSkinnedAnimation(float dt)
    {
        dt *= Speed;
        TimePos += dt;

        if (Loop)
        {
            float duration = animation->mDuration;
            if (TimePos > duration)
            {
                TimePos = 0.0;
            }
        }
        
        skeleton->GetTransforms(TimePos, rootNode, animation, aiMatrix4x4(), rootNode->mTransformation.Inverse(), transforms);

    }
};