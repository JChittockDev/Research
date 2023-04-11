#include "../EngineApp.h"

void EngineApp::BuildRenderAssets()
{
    BuildLights();
    BuildShaderPreRequisites();
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