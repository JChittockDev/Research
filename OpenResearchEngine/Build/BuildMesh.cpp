#include "../EngineApp.h"

void EngineApp::BuildMesh()
{
    std::string testFBX = "Models\\test.fbx";
    Mesh mesh(testFBX, "Take 001", md3dDevice, mCommandList, mGeometries, mSkinnedSubsets, mSkinnedMats);
    mesh.SkinnedCBIndex = SkinnedCBIndex++;
    mSkinnedMesh[testFBX] = mesh;
}