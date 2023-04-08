#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    int index = 0;
    for (auto controller : mAnimationControllers)
    {
        SkinnedConstants skinnedConstants;
        std::vector<DirectX::XMFLOAT4X4> transforms;
        mAnimationControllers[controller.first]->UpdateSkinnedAnimation(gt.DeltaTime());
        std::copy(std::begin(mAnimationControllers[controller.first]->transforms), std::end(mAnimationControllers[controller.first]->transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(index, skinnedConstants);
        index++;
    }
}