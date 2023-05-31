#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetVerletSolverRootSignature();
	SetTriangleNormalRootSignature();
	SetVertexNormalRootSignature();
	SetSsaoRootSignature();
}