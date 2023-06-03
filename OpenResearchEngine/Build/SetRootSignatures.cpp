#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetPreSolveRootSignature();
	SetVerletSolverRootSignature();
	SetPostSolveRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}