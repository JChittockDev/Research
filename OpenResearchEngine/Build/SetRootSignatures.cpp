#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetComputeRootSignature();
	SetSsaoRootSignature();
}