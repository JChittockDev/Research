#include "../EngineApp.h"

void EngineApp::BuildTextures()
{
    std::vector<std::string> texNames =
    {
        "bricksDiffuseMap",
        "bricksNormalMap",
        "tileDiffuseMap",
        "tileNormalMap",
        "defaultDiffuseMap",
        "defaultNormalMap",
        "skyCubeMap"
    };

    std::vector<std::wstring> texFilenames =
    {
        L"Textures/bricks2.dds",
        L"Textures/bricks2_nmap.dds",
        L"Textures/tile.dds",
        L"Textures/tile_nmap.dds",
        L"Textures/white1x1.dds",
        L"Textures/default_nmap.dds",
        L"Textures/desertcube1024.dds"
    };

    // Add skinned model textures to list so we can reference by name later.
    for (UINT i = 0; i < mSkinnedMats.size(); ++i)
    {
        std::string diffuseName = mSkinnedMats[i]->DiffuseMapName;
        std::string normalName = mSkinnedMats[i]->NormalMapName;

        std::wstring diffuseFilename = L"Textures/" + AnsiToWString(diffuseName);
        std::wstring normalFilename = L"Textures/" + AnsiToWString(normalName);

        // strip off extension
        diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
        normalName = normalName.substr(0, normalName.find_last_of("."));

        mSkinnedTextureNames.push_back(diffuseName);
        texNames.push_back(diffuseName);
        texFilenames.push_back(diffuseFilename);

        mSkinnedTextureNames.push_back(normalName);
        texNames.push_back(normalName);
        texFilenames.push_back(normalFilename);
    }

    for (int i = 0; i < (int)texNames.size(); ++i)
    {
        // Don't create duplicates.
        if (mTextures.find(texNames[i]) == std::end(mTextures))
        {
            auto texMap = std::make_unique<Texture>();
            texMap->Name = texNames[i];
            texMap->Filename = texFilenames[i];
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
                mCommandList.Get(), texMap->Filename.c_str(),
                texMap->Resource, texMap->UploadHeap));

            mTextures[texMap->Name] = std::move(texMap);
        }
    }
}