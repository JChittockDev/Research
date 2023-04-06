#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    int index = 0;
    for (auto mesh : mSkinnedMesh)
    {
        SkinnedConstants skinnedConstants;
        std::vector<DirectX::XMFLOAT4X4> transforms;
        mSkinnedMesh[mesh.first].UpdateSkinnedCB(gt.DeltaTime(), transforms);
        std::copy(std::begin(transforms), std::end(transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(index, skinnedConstants);
        index++;
    }
}