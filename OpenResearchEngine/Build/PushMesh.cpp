#include "../EngineApp.h"

void EngineApp::PushMesh()
{
    std::string filename = "Models\\test.fbx";
    std::shared_ptr<Mesh> skinnedMesh = std::make_shared<Mesh>(filename, md3dDevice, mCommandList, mGeometries, mSubsets, mSkeletons, mAnimations, mTransforms);
    mMesh[filename] = std::move(skinnedMesh);

	filename = "Models\\directionallight.obj";
	std::shared_ptr<Mesh> directionalLightMesh = std::make_shared<Mesh>(filename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(directionalLightMesh);

	filename = "Models\\spotlight.obj";
	std::shared_ptr<Mesh> spotLightMesh = std::make_shared<Mesh>(filename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(spotLightMesh);
}