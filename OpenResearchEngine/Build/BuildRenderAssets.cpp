#include "../EngineApp.h"

void EngineApp::BuildRenderAssets()
{
    BuildLights();
    BuildRenderPasses();
    BuildMesh();
    BuildTextures();
    BuildRootSignature();
    BuildSsaoRootSignature();
    BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildGenericGeometry();
    BuildMaterials();
    BuildLevel();
    BuildFrameResources();
    BuildPSOs();
}