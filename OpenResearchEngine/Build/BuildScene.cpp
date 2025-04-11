#include "../EngineApp.h"

void EngineApp::BuildScene()
{
    SerializeLevel();
    PushLights();
    PushMesh();
    ImportTextures();
    SetRootSignatures();
    CompileShaders();
    PushGenericMesh();
    PushMaterials();
    PushRenderItems();
    SetFenceResources();
    SetPipelineStates();
}