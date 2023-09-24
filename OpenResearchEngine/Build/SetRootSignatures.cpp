#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetForceRootSignature();
	SetPreSolveRootSignature();
	SetStretchConstraintSolveRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}