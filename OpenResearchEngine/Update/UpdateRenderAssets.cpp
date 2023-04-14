#include "../EngineApp.h"

void EngineApp::UpdateRenderAssets(const GameTimer& gt)
{
	UpdateLights(gt);
	UpdateObjectCBs(gt);
	UpdateSkinnedCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateShadowTransform(gt);
	UpdateShadowTransform2(gt);
	UpdateMainPassCB(gt);
	UpdateShadowPassCB(gt);
	UpdateShadowPassCB2(gt);
	UpdateSsaoCB(gt);
}