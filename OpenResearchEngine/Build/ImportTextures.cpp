#include "../EngineApp.h"

void EngineApp::ImportTextures()
{
    // Default Materials will init texture data arrays

    mTextureNames =
    {
        "bricksDiffuseMap",
        "bricksNormalMap",
        "tileDiffuseMap",
        "tileNormalMap",
        "defaultDiffuseMap",
        "defaultNormalMap"
    };

    std::vector<std::wstring> texFilenames =
    {
        L"Textures/bricks2.dds",
        L"Textures/bricks2_nmap.dds",
        L"Textures/tile.dds",
        L"Textures/tile_nmap.dds",
        L"Textures/default_diffuse.dds",
        L"Textures/default_nmap.dds"
    };

    mTextureCubeNames =
    {
        "skyCubeMap"
    };

    std::vector<std::wstring> texCubeFilenames =
    {
        L"Textures/desertcube1024.dds"
    };

    for (const auto& item : mLevelMaterials.at("DemoLevel"))
    {
        std::string diffuseName = extractFileName(item.second.diffuse_tex_path);
        std::wstring diffuseFilename = AnsiToWString(item.second.diffuse_tex_path);
        diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
        mTextureNames.push_back(diffuseName);
        texFilenames.push_back(diffuseFilename);

        std::string normalName = extractFileName(item.second.normal_tex_path);
        std::wstring normalFilename = AnsiToWString(item.second.normal_tex_path);
        normalName = normalName.substr(0, normalName.find_last_of("."));
        mTextureNames.push_back(normalName);
        texFilenames.push_back(normalFilename);

        std::string roughnessName = extractFileName(item.second.roughness_tex_path);
        std::wstring roughnessFilename = AnsiToWString(item.second.roughness_tex_path);
        roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
        mTextureNames.push_back(roughnessName);
        texFilenames.push_back(roughnessFilename);

        std::string metalnessName = extractFileName(item.second.metalness_tex_path);
        std::wstring metalnessFilename = AnsiToWString(item.second.metalness_tex_path);
        metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
        mTextureNames.push_back(metalnessName);
        texFilenames.push_back(metalnessFilename);

        std::string specularName = extractFileName(item.second.specular_tex_path);
        std::wstring specularFilename = AnsiToWString(item.second.specular_tex_path);
        specularName = specularName.substr(0, specularName.find_last_of("."));
        mTextureNames.push_back(specularName);
        texFilenames.push_back(specularFilename);

        std::string heightName = extractFileName(item.second.height_tex_path);
        std::wstring heightFilename = AnsiToWString(item.second.height_tex_path);
        heightName = heightName.substr(0, heightName.find_last_of("."));
        mTextureNames.push_back(heightName);
        texFilenames.push_back(heightFilename);

        std::string opacityName = extractFileName(item.second.opacity_tex_path);
        std::wstring opacityFilename = AnsiToWString(item.second.opacity_tex_path);
        opacityName = opacityName.substr(0, opacityName.find_last_of("."));
        mTextureNames.push_back(opacityName);
        texFilenames.push_back(opacityFilename);

        std::string occlusionName = extractFileName(item.second.occlusion_tex_path);
        std::wstring occlusionFilename = AnsiToWString(item.second.occlusion_tex_path);
        occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
        mTextureNames.push_back(occlusionName);
        texFilenames.push_back(occlusionFilename);

        std::string refractionName = extractFileName(item.second.refraction_tex_path);
        std::wstring refractionFilename = AnsiToWString(item.second.refraction_tex_path);
        refractionName = refractionName.substr(0, refractionName.find_last_of("."));
        mTextureNames.push_back(refractionName);
        texFilenames.push_back(refractionFilename);

        std::string emissiveName = extractFileName(item.second.emissive_tex_path);
        std::wstring emissiveFilename = AnsiToWString(item.second.emissive_tex_path);
        emissiveName = emissiveName.substr(0, emissiveName.find_last_of("."));
        mTextureNames.push_back(emissiveName);
        texFilenames.push_back(emissiveFilename);

        std::string subsurfaceName = extractFileName(item.second.subsurface_tex_path);
        std::wstring subsurfaceFilename = AnsiToWString(item.second.subsurface_tex_path);
        subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
        mTextureNames.push_back(subsurfaceName);
        texFilenames.push_back(subsurfaceFilename);
    }


    UINT textureIndex = 0;
    for (UINT i = 0; i < (UINT)mTextureNames.size(); ++i)
    {
        if (mTextures.find(mTextureNames[i]) == mTextures.end())
        {
            Texture texMap;
            texMap.Name = mTextureNames[i];
            texMap.Filename = texFilenames[i];
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), texMap.Filename.c_str(), texMap.Resource, texMap.UploadHeap));
            mTextures[texMap.Name] = std::move(std::make_shared<std::pair<Texture, UINT>> (std::make_pair(texMap, textureIndex)));
        }
        textureIndex += 1;
    }

    for (UINT i = 0; i < (UINT)mTextureCubeNames.size(); ++i)
    {
        if (mTextureCubes.find(mTextureCubeNames[i]) == mTextureCubes.end())
        {
            Texture texCubeMap;
            texCubeMap.Name = mTextureCubeNames[i];
            texCubeMap.Filename = texCubeFilenames[i];
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), texCubeMap.Filename.c_str(), texCubeMap.Resource, texCubeMap.UploadHeap));
            mTextureCubes[texCubeMap.Name] = std::move(std::make_shared<std::pair<Texture, UINT>>(std::make_pair(texCubeMap, textureIndex)));
        }
        textureIndex += 1;
    }
}