#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Render/Resources/FrameResource.h"
#include "../Assets/AssetManager.h"
#include "../Utilities/GameTimer.h"
#include <imgui.h>

void UpdateMaterialBuffer(const GameTimer& gt, AssetManager& assets, FrameResource* fr)
{
	ImGui::SeparatorText("Materials");

	auto currMaterialBuffer = fr->MaterialBuffer.get();
	for (auto& e : assets.mMaterials)
	{
		Material* mat = e.second.get();

		if (ImGui::TreeNode(mat->Name.c_str()))
		{
			ImGui::SliderFloat("Reflectance", &mat->Reflectance, 0.001f, 2.0f, "%.5f");
			ImGui::SliderFloat("Roughness", &mat->Roughness, 0.001f, 1.0f, "%.5f");
			ImGui::SliderFloat("Metalness", &mat->Metalness, 0.001f, 1.0f, "%.5f");
			ImGui::SliderFloat("Bump", &mat->Bump, 0.001f, 1.0f, "%.5f");

			mat->NumFramesDirty = gNumFrameResources;

			ImGui::TreePop();
		}

		if (mat->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matData;
			matData.Color = mat->Color;
			matData.Reflectance = mat->Reflectance;
			matData.Roughness = mat->Roughness;
			matData.Metalness = mat->Metalness;
			matData.Bump = mat->Bump;
			matData.Lit = mat->Lit;
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
			mat->NumFramesDirty--;
		}
	}
}
