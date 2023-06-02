#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetMeshTransferRootSignature();
	SetVerletSolverRootSignature();
	SetSimMeshTransferRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}