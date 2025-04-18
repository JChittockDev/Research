#include "../EngineApp.h"

void EngineApp::PushMaterials()
{
	UINT matIndex = 0;
	UINT srvHeapIndex =0;

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MaterialIndex = matIndex++;
	bricks0->DiffuseSrvHeapIndex = mTextures["bricks2"]->second;
	bricks0->NormalSrvHeapIndex = mTextures["bricks2_nmap"]->second;
	bricks0->RoughnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->MetalnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->SpecularSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->HeightSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->OpacitySrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->OcclusionSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->RefractionSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->EmissiveSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->SubsurfaceSrvHeapIndex = mTextures["default_diffuse"]->second;
	bricks0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	bricks0->Roughness = 0.5f;
	mMaterials["bricks0"] = std::move(bricks0);

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MaterialIndex = matIndex++;
	tile0->DiffuseSrvHeapIndex = mTextures["tile"]->second;
	tile0->NormalSrvHeapIndex = mTextures["tile_nmap"]->second;
	tile0->RoughnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->MetalnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->SpecularSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->HeightSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->OpacitySrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->OcclusionSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->RefractionSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->EmissiveSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->SubsurfaceSrvHeapIndex = mTextures["default_diffuse"]->second;
	tile0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile0->FresnelR0 = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	tile0->Roughness = 0.5f;
	mMaterials["tile0"] = std::move(tile0);

	auto mirror0 = std::make_unique<Material>();
	mirror0->Name = "mirror0";
	mirror0->MaterialIndex = matIndex++;
	mirror0->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
	mirror0->RoughnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->MetalnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->SpecularSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->HeightSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->OpacitySrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->OcclusionSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->RefractionSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->EmissiveSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->SubsurfaceSrvHeapIndex = mTextures["default_diffuse"]->second;
	mirror0->DiffuseAlbedo = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mirror0->FresnelR0 = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	mirror0->Roughness = 0.1f;
	mMaterials["mirror0"] = std::move(mirror0);

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MaterialIndex = matIndex++;
	sky->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
	sky->RoughnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->MetalnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->SpecularSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->HeightSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->OpacitySrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->OcclusionSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->RefractionSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->EmissiveSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->SubsurfaceSrvHeapIndex = mTextures["default_diffuse"]->second;
	sky->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	sky->Roughness = 0.5f;
	mMaterials["sky"] = std::move(sky);

	auto def = std::make_unique<Material>();
	def->Name = "default";
	def->MaterialIndex = matIndex++;
	def->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
	def->RoughnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->MetalnessSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->SpecularSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->HeightSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->OpacitySrvHeapIndex = mTextures["default_diffuse"]->second;
	def->OcclusionSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->RefractionSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->EmissiveSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->SubsurfaceSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->DiffuseAlbedo = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	def->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
	def->Roughness = 0.9f;
	mMaterials["default"] = std::move(def);

	for (const auto& item : mLevelMaterials.at("DemoLevel"))
	{
		auto mat = std::make_unique<Material>();
		mat->Name = item.first;
		mat->MaterialIndex = matIndex++;

		std::string diffuseName = extractFileName(item.second.diffuse_tex_path);
		diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
		mat->DiffuseSrvHeapIndex = mTextures[diffuseName]->second;

		std::string normalName = extractFileName(item.second.normal_tex_path);
		normalName = normalName.substr(0, normalName.find_last_of("."));
		mat->NormalSrvHeapIndex = mTextures[normalName]->second;

		std::string roughnessName = extractFileName(item.second.roughness_tex_path);
		roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
		mat->RoughnessSrvHeapIndex = mTextures[roughnessName]->second;

		std::string metalnessName = extractFileName(item.second.metalness_tex_path);
		metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
		mat->MetalnessSrvHeapIndex = mTextures[metalnessName]->second;

		std::string specularName = extractFileName(item.second.specular_tex_path);
		specularName = specularName.substr(0, specularName.find_last_of("."));
		mat->SpecularSrvHeapIndex = mTextures[specularName]->second;

		std::string heightName = extractFileName(item.second.height_tex_path);
		heightName = heightName.substr(0, heightName.find_last_of("."));
		mat->HeightSrvHeapIndex = mTextures[heightName]->second;

		std::string opacityName = extractFileName(item.second.opacity_tex_path);
		opacityName = opacityName.substr(0, opacityName.find_last_of("."));
		mat->OpacitySrvHeapIndex = mTextures[opacityName]->second;

		std::string occlusionName = extractFileName(item.second.occlusion_tex_path);
		occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
		mat->OcclusionSrvHeapIndex = mTextures[occlusionName]->second;

		std::string refractionName = extractFileName(item.second.refraction_tex_path);
		refractionName = refractionName.substr(0, refractionName.find_last_of("."));
		mat->RefractionSrvHeapIndex = mTextures[refractionName]->second;

		std::string emssiveName = extractFileName(item.second.emissive_tex_path);
		emssiveName = emssiveName.substr(0, emssiveName.find_last_of("."));
		mat->EmissiveSrvHeapIndex = mTextures[emssiveName]->second;

		std::string subsurfaceName = extractFileName(item.second.subsurface_tex_path);
		subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
		mat->SubsurfaceSrvHeapIndex = mTextures[subsurfaceName]->second;

		mat->DiffuseAlbedo = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
		mat->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
		mat->Roughness = 0.9f;
		
		mMaterials[mat->Name] = std::move(mat);
	}
}