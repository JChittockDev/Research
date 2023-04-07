#include "../EngineApp.h"

void EngineApp::BuildMesh()
{
    std::string testFBX = "Models\\test.fbx";
    Mesh mesh(testFBX, "mixamo.com", md3dDevice, mCommandList, mGeometries, mSkinnedSubsets, mSkinnedMats);
    mSkinnedMesh[testFBX] = mesh;
}