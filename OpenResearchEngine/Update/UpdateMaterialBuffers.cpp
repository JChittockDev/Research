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

			MaterialConstants matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat->NormalSrvHeapIndex;
			matData.RoughnessMapIndex = mat->DiffuseSrvHeapIndex;
			matData.MetalnessMapIndex = mat->MetalnessSrvHeapIndex;
			matData.SpecularMapIndex = mat->SpecularSrvHeapIndex;
			matData.HeightMapIndex = mat->HeightSrvHeapIndex;
			matData.OpacityMapIndex = mat->OpacitySrvHeapIndex;
			matData.OcclusionMapIndex = mat->OcclusionSrvHeapIndex;
			matData.RefractionMapIndex = mat->RefractionSrvHeapIndex;
			matData.EmissiveMapIndex = mat->EmissiveSrvHeapIndex;
			matData.SubsurfaceMapIndex = mat->SubsurfaceSrvHeapIndex;
			matData.ReflectionMapIndex = mat->ReflectionSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MaterialIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}