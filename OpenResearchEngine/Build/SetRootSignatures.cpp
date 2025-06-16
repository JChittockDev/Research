#include "../EngineApp.h"

void EngineApp::SetRootSignatures()
{
	SetGenericRootSignature();
	SetGBufferRootSignature();
	SetSsaoRootSignature();
	SetSsgiRootSignature();
	SetEdgeBlurRootSignature();
	SetColorEdgeBlurRootSignature();
	SetCompositeRootSignature();
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