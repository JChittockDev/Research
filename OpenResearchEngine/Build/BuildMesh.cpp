#include "../EngineApp.h"

void EngineApp::BuildMesh()
{
    Mesh mesh(mSkinnedModelFilename, "Take 001", md3dDevice, mCommandList, mGeometries, mSkinnedSubsets, mSkinnedMats);
    skinnedMesh.push_back(mesh);
}