#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    // We only have one skinned model being animated.
    mSkinnedModelInst->UpdateSkinnedAnimation(5.00);

    SkinnedConstants skinnedConstants;
    for (size_t i = 0; i < mSkinnedModelInst->FinalTransforms.size(); ++i)
    {       
        skinnedConstants.BoneTransforms[i] = mSkinnedModelInst->FinalTransforms[i];
    }

    currSkinnedCB->CopyData(0, skinnedConstants);
}