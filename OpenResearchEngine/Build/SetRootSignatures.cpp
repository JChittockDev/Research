#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetBlendRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetTensionRootSignature();
	SetForceRootSignature();
	SetPreSolveRootSignature();
	SetStretchConstraintSolveRootSignature();
	SetBendingConstraintSolveRootSignature();
	SetPostSolveRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}