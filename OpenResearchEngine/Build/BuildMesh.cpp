#include "../EngineApp.h"

void EngineApp::BuildMesh()
{
    std::string filename = "Models\\test.fbx";
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(filename, md3dDevice, mCommandList, mGeometries, mSkinnedSubsets, mSkinnedMats, mSkeletons, mAnimations, mTransforms);
    mSkinnedMesh[filename] = std::move(mesh);
}