#include "../EngineApp.h"

void EngineApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void EngineApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void EngineApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void EngineApp::OnKeyboardInput(const GameTimer& gt)
{
	bool imguiCapturingKeyboard = ImGui::GetIO().WantCaptureKeyboard;

	const float dt = gt.DeltaTime();

	if (!imguiCapturingKeyboard)
	{
		if (GetAsyncKeyState('W') & 0x8000)
			mCamera.Walk(10.0f * dt);

		if (GetAsyncKeyState('S') & 0x8000)
			mCamera.Walk(-10.0f * dt);

		if (GetAsyncKeyState('A') & 0x8000)
			mCamera.Strafe(-10.0f * dt);

		if (GetAsyncKeyState('D') & 0x8000)
			mCamera.Strafe(10.0f * dt);

		mCamera.UpdateViewMatrix();
	}
}