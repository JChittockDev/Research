#include "../EngineApp.h"

void EngineApp::BuildScene()
{
    SerializeLevel();
    PushLights();
    PushGenericMesh();
    PushMesh();
    ImportTextures();
    SetRootSignatures();
    CompileShaders();
    PushMaterials();
    PushRenderItems();
    SetFenceResources();
    SetPipelineStates();
}