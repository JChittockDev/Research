#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetGBufferRootSignature();
	SetBlendRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetTensionRootSignature();
	SetForceRootSignature();
	SetPreSolveRootSignature();
	SetConstraintSolveRootSignature();
	SetPostSolveRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}