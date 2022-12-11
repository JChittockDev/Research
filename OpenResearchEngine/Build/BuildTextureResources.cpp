#include "../EngineApp.h"

void EngineApp::BuildTextureResources()
{
	for (auto& [key, val] : textureDefinitions)
	{
		if (textureDefinitions.find(key) == std::end(textureDefinitions))
		{
			auto texureMap = std::make_unique<Texture>();
			texureMap->Name = key;
			texureMap->Filename = AnsiToWString(textureDefinitions[key]);
			ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
				mCommandList.Get(), texureMap->Filename.c_str(),
				texureMap->Resource, texureMap->UploadHeap));

			textureRenderAssets[texureMap->Name] = std::move(texureMap);
		}
	}
}