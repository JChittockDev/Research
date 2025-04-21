#include "../EngineApp.h"

void EngineApp::PushMesh()
{
    std::string filename = GetFullPath("Models/test.fbx");
	std::string vertexColorFilename = GetFullPath("Models/test.jpg");
    std::shared_ptr<Mesh> skinnedMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, mSkeletons, mAnimations, mTransforms);
    mMesh[filename] = std::move(skinnedMesh);

	filename = GetFullPath("Models/directionallight.obj");
	std::shared_ptr<Mesh> directionalLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(directionalLightMesh);

	filename = GetFullPath("Models/spotlight.obj");
	std::shared_ptr<Mesh> spotLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(spotLightMesh);
}