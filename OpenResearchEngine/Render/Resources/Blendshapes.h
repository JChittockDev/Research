#pragma once

#include "Skeleton.h"

class BlendshapeController
{
public:
    std::shared_ptr<Animation> animation = nullptr;
    std::map<std::string, std::vector<float>> weights;
    
    float TimePos = 0.0f;
    bool Loop = false;
    float Speed = 1.0f;

    void UpdateBlends(float dt)
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

        GetWeights(TimePos, animation, weights);
    }

    int FindWeightKey(float AnimationTime, std::unique_ptr<BlendAnimNode>& pNodeAnim)
    {
        for (int i = 0; i < pNodeAnim->weightKeys.size() - 1; i++) {
            if (AnimationTime < (float)pNodeAnim->weightKeys[i + 1].mTime) {
                return i;
            }
        }
        return 0;
    }

    void CalcInterpolatedWeights(float AnimationTime, std::unique_ptr<BlendAnimNode>& pNodeAnim, std::vector<float>& weights)
    {
        if (pNodeAnim->weightKeys.size() == 1) {

            for (UINT i = 0; i < pNodeAnim->weightKeys[0].mNumValuesAndWeights; ++i)
            {
                weights[pNodeAnim->weightKeys[0].mValues[i]] = (float)pNodeAnim->weightKeys[0].mWeights[i];
            }

            return;
        }

        int WeightIndex = FindWeightKey(AnimationTime, pNodeAnim);
        int NextWeightIndex = (WeightIndex + 1);

        float DeltaTime = (float)(pNodeAnim->weightKeys[NextWeightIndex].mTime - pNodeAnim->weightKeys[WeightIndex].mTime);
        float Factor = Math::Clamp((AnimationTime - (float)pNodeAnim->weightKeys[WeightIndex].mTime) / DeltaTime, 0.0f, 1.0f);

        for (UINT i = 0; i < pNodeAnim->weightKeys[WeightIndex].mNumValuesAndWeights; ++i)
        {
            const float& Start = (float)pNodeAnim->weightKeys[WeightIndex].mWeights[i];
            const float& End = (float)pNodeAnim->weightKeys[NextWeightIndex].mWeights[i];
            float Delta = End - Start;

            weights[pNodeAnim->weightKeys[WeightIndex].mValues[i]] = Start + Factor * Delta;
        }
    }

    void GetWeights(float timePos, std::shared_ptr<Animation>& animation, std::map<std::string, std::vector<float>>& weights)
    {
        for (auto it = animation->BlendAnimNodes.begin(); it != animation->BlendAnimNodes.end(); ++it)
        {
            if (weights[it->first].size() > 0)
            {
                CalcInterpolatedWeights(timePos, it->second, weights[it->first]);
            }
        }
    }
};