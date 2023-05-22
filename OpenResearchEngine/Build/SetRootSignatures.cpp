#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetSkinnedRootSignature();
	SetVerletSolverRootSignature();
	SetSsaoRootSignature();
}