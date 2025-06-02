#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetGBufferRootSignature();
	SetSsaoRootSignature();
	SetEdgeBlurRootSignature();
	SetShadowsRootSignature();
	SetLightingRootSignature();
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
}