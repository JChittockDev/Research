#include "../EngineApp.h"

void EngineApp::PushMaterials()
{
	UINT matIndex = 0;
	UINT srvHeapIndex =0;

	auto def = std::make_unique<Material>();
	def->Name = "default";
	def->MaterialIndex = matIndex++;
	def->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
	def->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
	def->RoughnessSrvHeapIndex = mTextures["default_roughness"]->second;
	def->MetalnessSrvHeapIndex = mTextures["default_metalness"]->second;
	def->SpecularSrvHeapIndex = mTextures["default_specular"]->second;
	def->HeightSrvHeapIndex = mTextures["default_height"]->second;
	def->OpacitySrvHeapIndex = mTextures["default_opacity"]->second;
	def->OcclusionSrvHeapIndex = mTextures["default_occlusion"]->second;
	def->RefractionSrvHeapIndex = mTextures["default_refraction"]->second;
	def->EmissiveSrvHeapIndex = mTextures["default_emissive"]->second;
	def->SubsurfaceSrvHeapIndex = mTextures["default_subsurface"]->second;
	def->ReflectionSrvHeapIndex = mTextures["default_reflection"]->second;
	def->DiffuseAlbedo = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	def->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
	def->Roughness = 0.9f;
	mMaterials["default"] = std::move(def);

	for (const auto& item : mLevelMaterials.at("DemoLevel"))
	{
		auto mat = std::make_unique<Material>();
		mat->Name = item.first;
		mat->MaterialIndex = matIndex++;


		if (!item.second.diffuse_tex_path.empty())
		{
			std::string diffuseName = extractFileName(item.second.diffuse_tex_path);
			diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
			mat->DiffuseSrvHeapIndex = mTextures[diffuseName]->second;
		}
		else
		{
			mat->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
		}

		if (!item.second.normal_tex_path.empty())
		{
			std::string normalName = extractFileName(item.second.normal_tex_path);
			normalName = normalName.substr(0, normalName.find_last_of("."));
			mat->NormalSrvHeapIndex = mTextures[normalName]->second;
		}
		else
		{
			mat->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
		}

		if (!item.second.roughness_tex_path.empty())
		{
			std::string roughnessName = extractFileName(item.second.roughness_tex_path);
			roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
			mat->RoughnessSrvHeapIndex = mTextures[roughnessName]->second;
		}
		else
		{
			mat->RoughnessSrvHeapIndex = mTextures["default_roughness"]->second;
		}

		if (!item.second.metalness_tex_path.empty())
		{
			std::string metalnessName = extractFileName(item.second.metalness_tex_path);
			metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
			mat->MetalnessSrvHeapIndex = mTextures[metalnessName]->second;
		}
		else
		{
			mat->MetalnessSrvHeapIndex = mTextures["default_metalness"]->second;
		}

		if (!item.second.specular_tex_path.empty())
		{
			std::string specularName = extractFileName(item.second.specular_tex_path);
			specularName = specularName.substr(0, specularName.find_last_of("."));
			mat->SpecularSrvHeapIndex = mTextures[specularName]->second;
		}
		else
		{
			mat->SpecularSrvHeapIndex = mTextures["default_specular"]->second;
		}

		if (!item.second.height_tex_path.empty())
		{
			std::string heightName = extractFileName(item.second.height_tex_path);
			heightName = heightName.substr(0, heightName.find_last_of("."));
			mat->HeightSrvHeapIndex = mTextures[heightName]->second;
		}
		else
		{
			mat->HeightSrvHeapIndex = mTextures["default_height"]->second;
		}

		if (!item.second.opacity_tex_path.empty())
		{
			std::string opacityName = extractFileName(item.second.opacity_tex_path);
			opacityName = opacityName.substr(0, opacityName.find_last_of("."));
			mat->OpacitySrvHeapIndex = mTextures[opacityName]->second;
		}
		else
		{
			mat->OpacitySrvHeapIndex = mTextures["default_opacity"]->second;
		}

		if (!item.second.occlusion_tex_path.empty())
		{
			std::string occlusionName = extractFileName(item.second.occlusion_tex_path);
			occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
			mat->OcclusionSrvHeapIndex = mTextures[occlusionName]->second;
		}
		else
		{
			mat->OcclusionSrvHeapIndex = mTextures["default_occlusion"]->second;
		}

		if (!item.second.refraction_tex_path.empty())
		{
			std::string refractionName = extractFileName(item.second.refraction_tex_path);
			refractionName = refractionName.substr(0, refractionName.find_last_of("."));
			mat->RefractionSrvHeapIndex = mTextures[refractionName]->second;
		}
		else
		{
			mat->RefractionSrvHeapIndex = mTextures["default_refraction"]->second;
		}

		if (!item.second.emissive_tex_path.empty())
		{
			std::string emssiveName = extractFileName(item.second.emissive_tex_path);
			emssiveName = emssiveName.substr(0, emssiveName.find_last_of("."));
			mat->EmissiveSrvHeapIndex = mTextures[emssiveName]->second;
		}
		else
		{
			mat->EmissiveSrvHeapIndex = mTextures["default_emissive"]->second;
		}

		if (!item.second.subsurface_tex_path.empty())
		{
			std::string subsurfaceName = extractFileName(item.second.subsurface_tex_path);
			subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
			mat->SubsurfaceSrvHeapIndex = mTextures[subsurfaceName]->second;
		}
		else
		{
			mat->SubsurfaceSrvHeapIndex = mTextures["default_subsurface"]->second;
		}

		if (!item.second.reflection_tex_path.empty())
		{
			std::string reflectionName = extractFileName(item.second.reflection_tex_path);
			reflectionName = reflectionName.substr(0, reflectionName.find_last_of("."));
			mat->ReflectionSrvHeapIndex = mTextures[reflectionName]->second;
		}
		else
		{
			mat->ReflectionSrvHeapIndex = mTextures["default_reflection"]->second;
		}

		if (item.second.tint.size() != 0)
		{
			mat->DiffuseAlbedo = DirectX::XMFLOAT4(item.second.tint[0], item.second.tint[1], item.second.tint[2], item.second.tint[3]);
		}
		else
		{
			mat->DiffuseAlbedo = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		}

		if (item.second.ref_coeff.size() != 0)
		{
			mat->FresnelR0 = DirectX::XMFLOAT3(item.second.ref_coeff[0], item.second.ref_coeff[0], item.second.ref_coeff[0]);
		}
		else
		{
			mat->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
		}

		if (item.second.rough_coeff != 0.0)
		{
			mat->Roughness = item.second.rough_coeff;
		}
		else
		{
			mat->Roughness = 0.9f;
		}

		mat->Lit = item.second.lit;
		
		mMaterials[mat->Name] = std::move(mat);
	}
}