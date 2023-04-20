#include "../EngineApp.h"

void EngineApp::BuildScene()
{
    SerializeLevel();
    PushLights();
    PushMesh();
    ImportTextures();
    SetRootSignatures();
    PopulateDescriptorHeaps();
    CompileShaders();
    PushGenericMesh();
    PushMaterials();
    PushRenderItems();
    SetFenceResources();
    SetPipelineStates();
}