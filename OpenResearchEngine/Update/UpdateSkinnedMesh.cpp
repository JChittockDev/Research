#include "../EngineApp.h"

void EngineApp::UpdateSkinnedCBs(const GameTimer& gt)
{
    SkinnedConstants skinnedConstants;
    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

    int transformCounter = 0;
    for (UINT x = 0; x < skinnedMesh.size(); ++x)
    {
        std::vector<DirectX::XMFLOAT4X4> transforms;
        skinnedMesh[x].UpdateSkinnedCB(gt.DeltaTime(), transforms);
        
        std::copy(std::begin(transforms), std::end(transforms), &skinnedConstants.BoneTransforms[transformCounter]);
        transformCounter += transforms.size();
    }

    currSkinnedCB->CopyData(0, skinnedConstants);
}