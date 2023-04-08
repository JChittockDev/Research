#include "../EngineApp.h"

void EngineApp::BuildRenderAssets()
{
    BuildMesh();
    BuildTextures();
    BuildRootSignature();
    BuildSsaoRootSignature();
    BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildGenericGeometry();
    BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();
}