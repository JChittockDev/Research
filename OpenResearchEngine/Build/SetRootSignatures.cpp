#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetForceRootSignature();
	SetPreSolveRootSignature();
	SetStretchConstraintSolveRootSignature();
	SetPostSolveRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}