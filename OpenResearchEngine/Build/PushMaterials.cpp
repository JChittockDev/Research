#include "../EngineApp.h"

void EngineApp::PushMaterials()
{
	UINT matCBIndex = 0;
	UINT srvHeapIndex =0;

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = matCBIndex++;
	bricks0->DiffuseSrvHeapIndex = mTextures["bricksDiffuseMap"]->second;
	bricks0->NormalSrvHeapIndex = mTextures["bricksNormalMap"]->second;
	bricks0->RoughnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->MetalnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->SpecularSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->HeightSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->OpacitySrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->OcclusionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->RefractionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->EmissiveSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->SubsurfaceSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	bricks0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks0->Roughness = 0.3f;
	mMaterials["bricks0"] = std::move(bricks0);

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = matCBIndex++;
	tile0->DiffuseSrvHeapIndex = mTextures["tileDiffuseMap"]->second;
	tile0->NormalSrvHeapIndex = mTextures["tileNormalMap"]->second;
	tile0->RoughnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->MetalnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->SpecularSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->HeightSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->OpacitySrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->OcclusionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->RefractionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->EmissiveSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->SubsurfaceSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	tile0->DiffuseAlbedo = DirectX::XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	tile0->FresnelR0 = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile0->Roughness = 0.1f;
	mMaterials["tile0"] = std::move(tile0);

	auto mirror0 = std::make_unique<Material>();
	mirror0->Name = "mirror0";
	mirror0->MatCBIndex = matCBIndex++;
	mirror0->DiffuseSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->NormalSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->RoughnessSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->MetalnessSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->SpecularSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->HeightSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->OpacitySrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->OcclusionSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->RefractionSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->EmissiveSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->SubsurfaceSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	mirror0->DiffuseAlbedo = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mirror0->FresnelR0 = DirectX::XMFLOAT3(0.98f, 0.97f, 0.95f);
	mirror0->Roughness = 0.1f;
	mMaterials["mirror0"] = std::move(mirror0);

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MatCBIndex = matCBIndex++;
	sky->DiffuseSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->NormalSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->RoughnessSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->MetalnessSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->SpecularSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->HeightSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->OpacitySrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->OcclusionSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->RefractionSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->EmissiveSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->SubsurfaceSrvHeapIndex = mTextureCubes["skyCubeMap"]->second;
	sky->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;
	mMaterials["sky"] = std::move(sky);

	auto def = std::make_unique<Material>();
	def->Name = "default";
	def->MatCBIndex = matCBIndex++;
	def->DiffuseSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->NormalSrvHeapIndex = mTextures["defaultNormalMap"]->second;
	def->RoughnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->MetalnessSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->SpecularSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->HeightSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->OpacitySrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->OcclusionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->RefractionSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->EmissiveSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->SubsurfaceSrvHeapIndex = mTextures["defaultDiffuseMap"]->second;
	def->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	def->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
	def->Roughness = 1.0f;
	mMaterials["default"] = std::move(def);

	for (const auto& item : mLevelMaterials.at("DemoLevel"))
	{
		auto mat = std::make_unique<Material>();
		mat->Name = item.first;
		mat->MatCBIndex = matCBIndex++;

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
		
		mMaterials[mat->Name] = std::move(mat);
	}
}