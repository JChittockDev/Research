#include "../EngineApp.h"

std::vector<Vector4> GetWeights(std::map<std::string, std::vector<float>>& weights)
{
    std::vector<Vector4> output;
    for (auto subset : weights)
    {
        for (int i = 0; i < subset.second.size(); i++)
        {
            Vector4 packedWeight;
            packedWeight.x = subset.second[i];
            output.push_back(packedWeight);
        }
    }
    return output;
}

void EngineApp::UpdateAnimCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();
    auto currBlendCB = mCurrFrameResource->BlendCB.get();

    int skinningIndex = 0;
    for (auto controller : mSkinningControllers)
    {
        SkinnedConstants skinnedConstants;
        mSkinningControllers.at(controller.first)->UpdateSkinning(gt.DeltaTime());
        std::copy(std::begin(mSkinningControllers.at(controller.first)->transforms), std::end(mSkinningControllers.at(controller.first)->transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(skinningIndex, skinnedConstants);
        skinningIndex++;
    }

    int blendIndex = 0;
    for (auto controller : mBlendshapeControllers)
    {
        float time = gt.TotalTime();
        BlendConstants blendConstants;
        mBlendshapeControllers.at(controller.first)->UpdateBlends(gt.DeltaTime());
        std::vector<Vector4> weights = GetWeights(mBlendshapeControllers.at(controller.first)->weights);
        std::copy(std::begin(weights), std::end(weights), &blendConstants.Weights[0]);
        currBlendCB->CopyData(blendIndex, blendConstants);
        blendIndex++;
    }

}