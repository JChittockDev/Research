#include "../EngineApp.h"

void EngineApp::ImportTextures()
{
    // Default Materials will init texture data arrays
    mTextureData = {
        {"default_diffuse", GetFullPath("Textures/default_diffuse.dds")},
        {"default_nmap",  GetFullPath("Textures/default_nmap.dds")},
        {"default_roughness",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_metalness",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_specular",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_height",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_opacity",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_occlusion",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_refraction",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_emissive",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_subsurface",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_reflection",  GetFullPath("Textures/desertcube1024.dds")}
    };

    for (const auto& item : mLevelMaterials.at("DemoLevel"))
    {
        if (!item.second.diffuse_tex_path.empty())
        {
            std::string diffuseName = extractFileName(item.second.diffuse_tex_path);
            diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
            mTextureData[diffuseName] = GetFullPath(item.second.diffuse_tex_path.c_str());
        }

        if (!item.second.normal_tex_path.empty())
        {
            std::string normalName = extractFileName(item.second.normal_tex_path);
            normalName = normalName.substr(0, normalName.find_last_of("."));
            mTextureData[normalName] = GetFullPath(item.second.normal_tex_path.c_str());
        }

        if (!item.second.roughness_tex_path.empty())
        {
            std::string roughnessName = extractFileName(item.second.roughness_tex_path);
            roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
            mTextureData[roughnessName] = GetFullPath(item.second.roughness_tex_path.c_str());
        }

        if (!item.second.metalness_tex_path.empty())
        {
            std::string metalnessName = extractFileName(item.second.metalness_tex_path);
            metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
            mTextureData[metalnessName] = GetFullPath(item.second.metalness_tex_path.c_str());
        }

        if (!item.second.specular_tex_path.empty())
        {
            std::string specularName = extractFileName(item.second.specular_tex_path);
            specularName = specularName.substr(0, specularName.find_last_of("."));
            mTextureData[specularName] = GetFullPath(item.second.specular_tex_path.c_str());
        }

        if (!item.second.height_tex_path.empty())
        {
            std::string heightName = extractFileName(item.second.height_tex_path);
            heightName = heightName.substr(0, heightName.find_last_of("."));
            mTextureData[heightName] = GetFullPath(item.second.height_tex_path.c_str());
        }

        if (!item.second.opacity_tex_path.empty())
        {
            std::string opacityName = extractFileName(item.second.opacity_tex_path);
            opacityName = opacityName.substr(0, opacityName.find_last_of("."));
            mTextureData[opacityName] = GetFullPath(item.second.opacity_tex_path.c_str());
        }

        if (!item.second.occlusion_tex_path.empty())
        {
            std::string occlusionName = extractFileName(item.second.occlusion_tex_path);
            occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
            mTextureData[occlusionName] = GetFullPath(item.second.occlusion_tex_path.c_str());

        }

        if (!item.second.refraction_tex_path.empty())
        {
            std::string refractionName = extractFileName(item.second.refraction_tex_path);
            refractionName = refractionName.substr(0, refractionName.find_last_of("."));
            mTextureData[refractionName] = GetFullPath(item.second.refraction_tex_path.c_str());
        }

        if (!item.second.emissive_tex_path.empty())
        {
            std::string emissiveName = extractFileName(item.second.emissive_tex_path);
            emissiveName = emissiveName.substr(0, emissiveName.find_last_of("."));
            mTextureData[emissiveName] = GetFullPath(item.second.emissive_tex_path.c_str());
        }

        if (!item.second.subsurface_tex_path.empty())
        {
            std::string subsurfaceName = extractFileName(item.second.subsurface_tex_path);
            subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
            mTextureData[subsurfaceName] = GetFullPath(item.second.subsurface_tex_path.c_str());
        }

        if (!item.second.reflection_tex_path.empty())
        {
            std::string reflectionName = extractFileName(item.second.reflection_tex_path);
            reflectionName = reflectionName.substr(0, reflectionName.find_last_of("."));
            mTextureData[reflectionName] = GetFullPath(item.second.reflection_tex_path.c_str());
        }
    }

    UINT textureIndex = 0;
    for (const auto& texture : mTextureData)
    {
        if (mTextures.find(texture.first) == mTextures.end())
        {
            Texture texMap;
            texMap.Name = texture.first;
            texMap.Filename = AnsiToWString(texture.second);
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), texMap.Filename.c_str(), texMap.Resource, texMap.UploadHeap));
            mTextures[texMap.Name] = std::move(std::make_shared<std::pair<Texture, UINT>>(std::make_pair(texMap, textureIndex)));
        }

        textureIndex += 1;
    }
}