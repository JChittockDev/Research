#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    int index = 0;
    for (auto controller : mAnimationControllers)
    {
        SkinnedConstants skinnedConstants;
        std::vector<DirectX::XMFLOAT4X4> transforms;
        mAnimationControllers.at(controller.first)->UpdateSkinnedAnimation(gt.DeltaTime());
        std::copy(std::begin(mAnimationControllers.at(controller.first)->transforms), std::end(mAnimationControllers.at(controller.first)->transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(index, skinnedConstants);
        index++;
    }
}