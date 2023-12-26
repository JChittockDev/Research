#include "../EngineApp.h"

void EngineApp::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat->NormalSrvHeapIndex;
			matData.RoughnessMapIndex = mat->DiffuseSrvHeapIndex;
			matData.MetalnesssMapIndex = mat->MetalnessSrvHeapIndex;
			matData.SpecularsMapIndex = mat->SpecularSrvHeapIndex;
			matData.HeightsMapIndex = mat->HeightSrvHeapIndex;
			matData.OpacitysMapIndex = mat->OpacitySrvHeapIndex;
			matData.OcclusionsMapIndex = mat->OcclusionSrvHeapIndex;
			matData.RefractionsMapIndex = mat->RefractionSrvHeapIndex;
			matData.EmissivesMapIndex = mat->EmissiveSrvHeapIndex;
			matData.SubsurfacesMapIndex = mat->SubsurfaceSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MaterialIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}