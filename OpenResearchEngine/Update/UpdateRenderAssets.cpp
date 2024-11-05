#include "../EngineApp.h"

void EngineApp::UpdateRenderAssets(const GameTimer& gt)
{
	ImGui::Begin("Scene");
	UpdateLights(gt);
	UpdateObjectCBs(gt);
	UpdateAnimCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateShadowTransform(gt);
	UpdateMainPassCB(gt);
	UpdateShadowPassCB(gt);
	UpdateSsaoCB(gt);
	ImGui::End();
}