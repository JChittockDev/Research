#include "UpdateFunctions.h"
#include "../Objects/Camera.h"
#include "../Common/Structures.h"
#include "../Common/SceneState.h"
#include "../Render/Resources/FrameResource.h"
#include "../Utilities/GameTimer.h"
#include <DirectXMath.h>

void UpdateMainPassCB(const GameTimer& gt, const Camera& camera, SceneState& state, FrameResource* fr)
{
	DirectX::XMMATRIX view = camera.GetView();
	DirectX::XMMATRIX proj = camera.GetProj();

	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	DirectX::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

	DirectX::XMMATRIX viewProjTex = XMMatrixMultiply(viewProj, T);

	XMStoreFloat4x4(&state.mainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&state.mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&state.mainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&state.mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&state.mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&state.mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&state.mainPassCB.ViewProjTex, XMMatrixTranspose(viewProjTex));

	state.mainPassCB.EyePosW = camera.GetPosition3f();
	state.mainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)state.clientWidth, (float)state.clientHeight);
	state.mainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / state.clientWidth, 1.0f / state.clientHeight);
	state.mainPassCB.NearZ = 1.0f;
	state.mainPassCB.FarZ = 1000.0f;
	state.mainPassCB.TotalTime = gt.TotalTime();
	state.mainPassCB.DeltaTime = gt.DeltaTime();
	state.mainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	fr->PassCB->CopyData(0, state.mainPassCB);
}
