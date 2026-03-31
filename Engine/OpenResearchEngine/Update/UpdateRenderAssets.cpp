#include "UpdateFunctions.h"
#include "../EngineApp.h"

void EngineApp::UpdateRenderAssets(const GameTimer& gt)
{
	ImGui::Begin("Scene");
	mSceneState.clientWidth  = mClientWidth;
	mSceneState.clientHeight = mClientHeight;
	UpdateLights(gt, mSceneState, *mAssets, mCurrFrameResource);
	UpdateObjectCBs(gt, *mAssets, mCurrFrameResource);
	UpdateMaterialBuffer(gt, *mAssets, mCurrFrameResource);
	UpdateShadowTransform(gt, mSceneState);
	UpdateMainPassCB(gt, mCamera, mSceneState, mCurrFrameResource);
	UpdateSssCB(gt, mSceneState, mCurrFrameResource);
	UpdateScreenSpaceCB(gt, mSceneState, mSsao.get(), mSsgi.get(), mSss.get(), mCurrFrameResource);
	UpdateShadowPassCB(gt, mSceneState, mShadowResources.get(), mCurrFrameResource);
	UpdateRadiancePassCB(gt, mSceneState, *mAssets, mCurrFrameResource);
	ImGui::End();
}
