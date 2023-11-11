#pragma once

#include "Skeleton.h"

class SkinningController
{
public:
    std::shared_ptr<TransformNode> rootNode = nullptr;
    std::shared_ptr<Skeleton> skeleton = nullptr;
    std::shared_ptr<Animation> animation = nullptr;
    std::vector<DirectX::XMFLOAT4X4> transforms;
    
    float TimePos = 0.0f;
    bool Loop = true;
    float Speed = 1.0f;

    void UpdateSkinning(float dt)
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
        
        skeleton->GetTransforms(TimePos, rootNode, animation, DirectX::XMMatrixIdentity(), DirectX::XMMatrixInverse(nullptr, rootNode->transform), transforms);
    }
};