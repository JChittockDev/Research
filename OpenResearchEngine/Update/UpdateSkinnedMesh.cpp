#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    for (UINT x = 0; x < skinnedMesh.size(); ++x)
    {
        SkinnedConstants skinnedConstants;
        std::vector<DirectX::XMFLOAT4X4> transforms;
        skinnedMesh[x].UpdateSkinnedCB(gt.DeltaTime(), transforms);
        std::copy(std::begin(transforms), std::end(transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(x, skinnedConstants);
    }
}