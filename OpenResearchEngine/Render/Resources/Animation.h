#pragma once

#include "Skeleton.h"

class AnimationController
{
public:
    std::shared_ptr<Skeleton> skeleton = nullptr;
    std::shared_ptr<Animation> animation = nullptr;
    std::shared_ptr<TransformNode> rooNode = nullptr;
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
            float duration = animation->duration;
            if (TimePos > duration)
            {
                TimePos = 0.0;
            }
        }
        
        skeleton->GetTransforms(TimePos, rooNode, animation, DirectX::XMMatrixIdentity(), DirectX::XMMatrixInverse(nullptr, rooNode->transform), transforms);

    }
};