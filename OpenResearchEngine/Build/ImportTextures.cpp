#include "../EngineApp.h"

void EngineApp::ImportTextures()
{
    // Default Materials will init texture data arrays
    mTextureData = {
    {"bricks2" , "Textures/bricks2.dds"},
    {"bricks2_nmap", "Textures/bricks2_nmap.dds"},
    {"tile", "Textures/tile.dds"},
    {"tile_nmap", "Textures/tile_nmap.dds"},
    {"default_diffuse", "Textures/default_diffuse.dds"},
    {"default_nmap", "Textures/default_nmap.dds"},
    {"desertcube1024", "Textures/desertcube1024.dds"}
    };

    for (const auto& item : mLevelMaterials.at("DemoLevel"))
    {
        std::string diffuseName = extractFileName(item.second.diffuse_tex_path);
        diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
        mTextureData[diffuseName] = item.second.diffuse_tex_path;

        std::string normalName = extractFileName(item.second.normal_tex_path);
        normalName = normalName.substr(0, normalName.find_last_of("."));
        mTextureData[normalName] = item.second.normal_tex_path;

        std::string roughnessName = extractFileName(item.second.roughness_tex_path);
        roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
        mTextureData[roughnessName] = item.second.roughness_tex_path;

        std::string metalnessName = extractFileName(item.second.metalness_tex_path);
        metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
        mTextureData[metalnessName] = item.second.metalness_tex_path;

        std::string specularName = extractFileName(item.second.specular_tex_path);
        specularName = specularName.substr(0, specularName.find_last_of("."));
        mTextureData[specularName] = item.second.specular_tex_path;

        std::string heightName = extractFileName(item.second.height_tex_path);
        heightName = heightName.substr(0, heightName.find_last_of("."));
        mTextureData[heightName] = item.second.height_tex_path;

        std::string opacityName = extractFileName(item.second.opacity_tex_path);
        opacityName = opacityName.substr(0, opacityName.find_last_of("."));
        mTextureData[opacityName] = item.second.opacity_tex_path;

        std::string occlusionName = extractFileName(item.second.occlusion_tex_path);
        occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
        mTextureData[occlusionName] = item.second.occlusion_tex_path;

        std::string refractionName = extractFileName(item.second.refraction_tex_path);
        refractionName = refractionName.substr(0, refractionName.find_last_of("."));
        mTextureData[refractionName] = item.second.refraction_tex_path;

        std::string emissiveName = extractFileName(item.second.emissive_tex_path);
        emissiveName = emissiveName.substr(0, emissiveName.find_last_of("."));
        mTextureData[emissiveName] = item.second.emissive_tex_path;

        std::string subsurfaceName = extractFileName(item.second.subsurface_tex_path);
        subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
        mTextureData[subsurfaceName] = item.second.subsurface_tex_path;
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