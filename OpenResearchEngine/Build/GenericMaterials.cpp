#include "../EngineApp.h"

std::unique_ptr<MeshMaterial> EngineApp::GenericMaterials()
{

	std::unique_ptr<MeshMaterial> meshMaterials = std::make_unique<MeshMaterial>();

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = matCBIndex++;
	bricks0->DiffuseSrvHeapIndex = diffuseSrvHeapIndex++;
	bricks0->NormalSrvHeapIndex = normalSrvHeapIndex++;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks0->Roughness = 0.3f;
	meshMaterials->mappedMaterials[bricks0->Name] = std::move(bricks0);

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = matCBIndex++;
	tile0->DiffuseSrvHeapIndex = diffuseSrvHeapIndex++;
	tile0->NormalSrvHeapIndex = normalSrvHeapIndex++;
	tile0->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile0->Roughness = 0.1f;
	meshMaterials->mappedMaterials[tile0->Name] = std::move(tile0);


	auto mirror0 = std::make_unique<Material>();
	mirror0->Name = "mirror0";
	mirror0->MatCBIndex = matCBIndex++;
	mirror0->DiffuseSrvHeapIndex = diffuseSrvHeapIndex++;
	mirror0->NormalSrvHeapIndex = normalSrvHeapIndex++;
	mirror0->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mirror0->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	mirror0->Roughness = 0.1f;
	meshMaterials->mappedMaterials[mirror0->Name] = std::move(mirror0);

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MatCBIndex = matCBIndex++;
	sky->DiffuseSrvHeapIndex = diffuseSrvHeapIndex++;
	sky->NormalSrvHeapIndex = normalSrvHeapIndex++;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;
	meshMaterials->mappedMaterials[sky->Name] = std::move(sky);

	return meshMaterials;
}