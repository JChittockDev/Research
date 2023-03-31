#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    // We only have one skinned model being animated.
    mSkinnedModelInst->UpdateSkinnedAnimation(gt.DeltaTime());

    SkinnedConstants skinnedConstants;
    for (size_t i = 0; i < mSkinnedModelInst->FinalTransforms.size(); ++i, ++i)
    {
        skinnedConstants.BoneTransforms[i] = mSkinnedModelInst->FinalTransforms[i];
    }

    currSkinnedCB->CopyData(0, skinnedConstants);
}