#include "../EngineApp.h"

void EngineApp::PushMesh()
{
	std::string vertexColorFilename;
	std::string filename = GetFullPath("Models/directionallight.obj");
	std::shared_ptr<Mesh> directionalLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(directionalLightMesh);

	filename = GetFullPath("Models/spotlight.obj");
	std::shared_ptr<Mesh> spotLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(spotLightMesh);

	for (const auto& item : mLevelRenderItems.at("DemoLevel"))
	{
		const std::string& itemName = item.first;
		const ItemData& renderItemData = item.second;

		filename = GetFullPath(renderItemData.geometry.c_str());

		bool geoExists = PathExists(renderItemData.geometry);

		if (geoExists)
		{
			std::string texFileName = ChangeExtension(filename, ".jpg");

			bool texExists = PathExists(texFileName);

			if (texExists)
			{
				vertexColorFilename = texFileName;
			}

			std::shared_ptr<Mesh> skinnedMesh = std::make_shared<Mesh>(filename, vertexColorFilename, md3dDevice, mCommandList, mGeometries, mSubsets, mSkeletons, mAnimations, mTransforms);
			mMesh[filename] = std::move(skinnedMesh);
		}
	}
}