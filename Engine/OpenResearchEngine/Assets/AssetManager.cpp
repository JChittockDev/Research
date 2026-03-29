#include "AssetManager.h"

AssetManager::AssetManager(
    Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
    std::filesystem::path basePath,
    bool msaaState, UINT msaaQuality,
    DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
    : mDevice(device), mCmdList(cmdList), mBasePath(std::move(basePath)),
      mMsaaState(msaaState), mMsaaQuality(msaaQuality),
      mBackBufferFormat(backBufferFormat), mDepthStencilFormat(depthStencilFormat)
{}

void AssetManager::Build(
    const DynamicLights& lights,
    const std::unordered_map<std::string, std::unordered_map<std::string, ItemData>>& levelItems,
    const std::unordered_map<std::string, std::unordered_map<std::string, PBRMaterialData>>& levelMaterials,
    const std::unordered_map<std::string, std::unordered_map<std::string, LightData>>& levelLights)
{
    SetRootSignatures();
    CompileShaders(lights);
    ImportTextures(levelMaterials);
    PushGenericMesh();
    PushMesh(levelItems);
    PushMaterials(levelMaterials);
    SetPipelineStates();
}

std::string AssetManager::ExtractFileName(const std::string& filePath) {
    size_t found = filePath.find_last_of("/\\");
    return (found != std::string::npos) ? filePath.substr(found + 1) : filePath;
}

void AssetManager::ImportTextures(
    const std::unordered_map<std::string,
          std::unordered_map<std::string, PBRMaterialData>>& levelMaterials)
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

    for (const auto& item : levelMaterials.at("DemoLevel"))
    {
        if (!item.second.diffuse_tex_path.empty())
        {
            std::string diffuseName = ExtractFileName(item.second.diffuse_tex_path);
            diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
            mTextureData[diffuseName] = GetFullPath(item.second.diffuse_tex_path.c_str());
        }

        if (!item.second.normal_tex_path.empty())
        {
            std::string normalName = ExtractFileName(item.second.normal_tex_path);
            normalName = normalName.substr(0, normalName.find_last_of("."));
            mTextureData[normalName] = GetFullPath(item.second.normal_tex_path.c_str());
        }

        if (!item.second.roughness_tex_path.empty())
        {
            std::string roughnessName = ExtractFileName(item.second.roughness_tex_path);
            roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
            mTextureData[roughnessName] = GetFullPath(item.second.roughness_tex_path.c_str());
        }

        if (!item.second.metalness_tex_path.empty())
        {
            std::string metalnessName = ExtractFileName(item.second.metalness_tex_path);
            metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
            mTextureData[metalnessName] = GetFullPath(item.second.metalness_tex_path.c_str());
        }

        if (!item.second.specular_tex_path.empty())
        {
            std::string specularName = ExtractFileName(item.second.specular_tex_path);
            specularName = specularName.substr(0, specularName.find_last_of("."));
            mTextureData[specularName] = GetFullPath(item.second.specular_tex_path.c_str());
        }

        if (!item.second.height_tex_path.empty())
        {
            std::string heightName = ExtractFileName(item.second.height_tex_path);
            heightName = heightName.substr(0, heightName.find_last_of("."));
            mTextureData[heightName] = GetFullPath(item.second.height_tex_path.c_str());
        }

        if (!item.second.opacity_tex_path.empty())
        {
            std::string opacityName = ExtractFileName(item.second.opacity_tex_path);
            opacityName = opacityName.substr(0, opacityName.find_last_of("."));
            mTextureData[opacityName] = GetFullPath(item.second.opacity_tex_path.c_str());
        }

        if (!item.second.occlusion_tex_path.empty())
        {
            std::string occlusionName = ExtractFileName(item.second.occlusion_tex_path);
            occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
            mTextureData[occlusionName] = GetFullPath(item.second.occlusion_tex_path.c_str());

        }

        if (!item.second.refraction_tex_path.empty())
        {
            std::string refractionName = ExtractFileName(item.second.refraction_tex_path);
            refractionName = refractionName.substr(0, refractionName.find_last_of("."));
            mTextureData[refractionName] = GetFullPath(item.second.refraction_tex_path.c_str());
        }

        if (!item.second.emissive_tex_path.empty())
        {
            std::string emissiveName = ExtractFileName(item.second.emissive_tex_path);
            emissiveName = emissiveName.substr(0, emissiveName.find_last_of("."));
            mTextureData[emissiveName] = GetFullPath(item.second.emissive_tex_path.c_str());
        }

        if (!item.second.subsurface_tex_path.empty())
        {
            std::string subsurfaceName = ExtractFileName(item.second.subsurface_tex_path);
            subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
            mTextureData[subsurfaceName] = GetFullPath(item.second.subsurface_tex_path.c_str());
        }

        if (!item.second.reflection_tex_path.empty())
        {
            std::string reflectionName = ExtractFileName(item.second.reflection_tex_path);
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
            ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(mDevice.Get(), mCmdList.Get(), texMap.Filename.c_str(), texMap.Resource, texMap.UploadHeap));
            mTextures[texMap.Name] = std::move(std::make_shared<std::pair<Texture, UINT>>(std::make_pair(texMap, textureIndex)));
        }

        textureIndex += 1;
    }
}

void AssetManager::PushGenericMesh()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();

	UINT matIndex = 0;
	std::shared_ptr<Subset> boxSubset = std::make_shared<Subset>();
	boxSubset->VertexStart = boxVertexOffset;
	boxSubset->VertexCount = (UINT)box.Vertices.size();
	boxSubset->IndexStart = boxIndexOffset;
	boxSubset->IndexCount = (UINT)box.Indices32.size();
	boxSubset->TriangleStart = boxIndexOffset / 3;
	boxSubset->TriangleCount = (UINT)box.Indices32.size() / 3;
	boxSubset->Name = "box";

	std::shared_ptr<Subset> gridSubset = std::make_shared<Subset>();
	gridSubset->VertexStart = gridVertexOffset;
	gridSubset->VertexCount = (UINT)grid.Vertices.size();
	gridSubset->IndexStart = gridIndexOffset;
	gridSubset->IndexCount = (UINT)grid.Indices32.size();
	gridSubset->TriangleStart = gridIndexOffset / 3;
	gridSubset->TriangleCount = (UINT)grid.Indices32.size() / 3;
	gridSubset->Name = "grid";

	std::shared_ptr<Subset> sphereSubset = std::make_shared<Subset>();
	sphereSubset->VertexStart = sphereVertexOffset;
	sphereSubset->VertexCount = (UINT)sphere.Vertices.size();
	sphereSubset->IndexStart = sphereIndexOffset;
	sphereSubset->IndexCount = (UINT)sphere.Indices32.size();
	sphereSubset->TriangleStart = sphereIndexOffset / 3;
	sphereSubset->TriangleCount = (UINT)sphere.Indices32.size() / 3;
	sphereSubset->Name = "sphere";

	std::shared_ptr<Subset> cylinderSubset = std::make_shared<Subset>();
	cylinderSubset->VertexStart = cylinderVertexOffset;
	cylinderSubset->VertexCount = (UINT)cylinder.Vertices.size();
	cylinderSubset->IndexStart = cylinderIndexOffset;
	cylinderSubset->IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubset->TriangleStart = cylinderIndexOffset / 3;
	cylinderSubset->TriangleCount = (UINT)cylinder.Indices32.size() / 3;
	cylinderSubset->Name = "cylinder";

	std::shared_ptr<Subset> quadSubset = std::make_shared<Subset>();
	quadSubset->VertexStart = quadVertexOffset;
	quadSubset->VertexCount = (UINT)quad.Vertices.size();
	quadSubset->IndexStart = quadIndexOffset;
	quadSubset->IndexCount = (UINT)quad.Indices32.size();
	quadSubset->TriangleStart = quadIndexOffset / 3;
	quadSubset->TriangleCount = (UINT)quad.Indices32.size() / 3;
	quadSubset->Name = "quad";

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = boxSubset->IndexCount;
	boxSubmesh.IndexStart = boxSubset->IndexStart;
	boxSubmesh.VertexCount = boxSubset->VertexCount;
	boxSubmesh.VertexStart = boxSubset->VertexStart;
	boxSubmesh.TriangleCount = boxSubset->TriangleCount;
	boxSubmesh.TriangleStart = boxSubset->TriangleStart;
	boxSubmesh.Name = boxSubset->Name;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = gridSubset->IndexCount;
	gridSubmesh.IndexStart = gridSubset->IndexStart;
	gridSubmesh.VertexCount = gridSubset->VertexCount;
	gridSubmesh.VertexStart = gridSubset->VertexStart;
	gridSubmesh.TriangleCount = gridSubset->TriangleCount;
	gridSubmesh.TriangleStart = gridSubset->TriangleStart;
	gridSubmesh.Name = gridSubset->Name;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = sphereSubset->IndexCount;
	sphereSubmesh.IndexStart = sphereSubset->IndexStart;
	sphereSubmesh.VertexCount = sphereSubset->VertexCount;
	sphereSubmesh.VertexStart = sphereSubset->VertexStart;
	sphereSubmesh.TriangleCount = sphereSubset->TriangleCount;
	sphereSubmesh.TriangleStart = sphereSubset->TriangleStart;
	sphereSubmesh.Name = sphereSubset->Name;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = cylinderSubset->IndexCount;
	cylinderSubmesh.IndexStart = cylinderSubset->IndexStart;
	cylinderSubmesh.VertexCount = cylinderSubset->VertexCount;
	cylinderSubmesh.VertexStart = cylinderSubset->VertexStart;
	cylinderSubmesh.TriangleCount = cylinderSubset->TriangleCount;
	cylinderSubmesh.TriangleStart = cylinderSubset->TriangleStart;
	cylinderSubmesh.Name = cylinderSubset->Name;

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = quadSubset->IndexCount;
	quadSubmesh.IndexStart = quadSubset->IndexStart;
	quadSubmesh.VertexCount = quadSubset->VertexCount;
	quadSubmesh.VertexStart = quadSubset->VertexStart;
	quadSubmesh.TriangleCount = quadSubset->TriangleCount;
	quadSubmesh.TriangleStart = quadSubset->TriangleStart;
	quadSubmesh.Name = quadSubset->Name;

	mSubsets["shapeGeo"].push_back(std::move(boxSubset));
	mSubsets["shapeGeo"].push_back(std::move(gridSubset));
	mSubsets["shapeGeo"].push_back(std::move(sphereSubset));
	mSubsets["shapeGeo"].push_back(std::move(cylinderSubset));
	mSubsets["shapeGeo"].push_back(std::move(quadSubset));


	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(box.Vertices[i].TangentU.x, box.Vertices[i].TangentU.y, box.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(grid.Vertices[i].TangentU.x, grid.Vertices[i].TangentU.y, grid.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(sphere.Vertices[i].TangentU.x, sphere.Vertices[i].TangentU.y, sphere.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(cylinder.Vertices[i].TangentU.x, cylinder.Vertices[i].TangentU.y, cylinder.Vertices[i].TangentU.z, 1.0);
	}

	for (int i = 0; i < quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = quad.Vertices[i].Position;
		vertices[k].Normal = quad.Vertices[i].Normal;
		vertices[k].TexC = quad.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(quad.Vertices[i].TangentU.x, quad.Vertices[i].TangentU.y, quad.Vertices[i].TangentU.z, 1.0);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(mDevice.Get(),
		mCmdList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(mDevice.Get(),
		mCmdList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void AssetManager::PushMesh(
    const std::unordered_map<std::string,
          std::unordered_map<std::string, ItemData>>& levelItems)
{
	std::string vertexColorFilename;
	std::string filename = GetFullPath("Models/directionallight.obj");
	std::shared_ptr<Mesh> directionalLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, mDevice, mCmdList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(directionalLightMesh);

	filename = GetFullPath("Models/spotlight.obj");
	std::shared_ptr<Mesh> spotLightMesh = std::make_shared<Mesh>(filename, vertexColorFilename, mDevice, mCmdList, mGeometries, mSubsets, true);
	mMesh[filename] = std::move(spotLightMesh);

	for (const auto& item : levelItems.at("DemoLevel"))
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

			std::shared_ptr<Mesh> skinnedMesh = std::make_shared<Mesh>(filename, vertexColorFilename, mDevice, mCmdList, mGeometries, mSubsets, mSkeletons, mAnimations, mTransforms);
			mMesh[filename] = std::move(skinnedMesh);
		}
	}
}

void AssetManager::PushMaterials(
    const std::unordered_map<std::string,
          std::unordered_map<std::string, PBRMaterialData>>& levelMaterials)
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
	def->Color = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	def->Reflectance = 0.04f;
	def->Roughness = 0.5f;
	def->Metalness = 0.0f;
	mMaterials["default"] = std::move(def);

	for (const auto& item : levelMaterials.at("DemoLevel"))
	{
		auto mat = std::make_unique<Material>();
		mat->Name = item.first;
		mat->MaterialIndex = matIndex++;


		if (!item.second.diffuse_tex_path.empty())
		{
			std::string diffuseName = ExtractFileName(item.second.diffuse_tex_path);
			diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
			mat->DiffuseSrvHeapIndex = mTextures[diffuseName]->second;
		}
		else
		{
			mat->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second;
		}

		if (!item.second.normal_tex_path.empty())
		{
			std::string normalName = ExtractFileName(item.second.normal_tex_path);
			normalName = normalName.substr(0, normalName.find_last_of("."));
			mat->NormalSrvHeapIndex = mTextures[normalName]->second;
		}
		else
		{
			mat->NormalSrvHeapIndex = mTextures["default_nmap"]->second;
		}

		if (!item.second.roughness_tex_path.empty())
		{
			std::string roughnessName = ExtractFileName(item.second.roughness_tex_path);
			roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
			mat->RoughnessSrvHeapIndex = mTextures[roughnessName]->second;
		}
		else
		{
			mat->RoughnessSrvHeapIndex = mTextures["default_roughness"]->second;
		}

		if (!item.second.metalness_tex_path.empty())
		{
			std::string metalnessName = ExtractFileName(item.second.metalness_tex_path);
			metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
			mat->MetalnessSrvHeapIndex = mTextures[metalnessName]->second;
		}
		else
		{
			mat->MetalnessSrvHeapIndex = mTextures["default_metalness"]->second;
		}

		if (!item.second.specular_tex_path.empty())
		{
			std::string specularName = ExtractFileName(item.second.specular_tex_path);
			specularName = specularName.substr(0, specularName.find_last_of("."));
			mat->SpecularSrvHeapIndex = mTextures[specularName]->second;
		}
		else
		{
			mat->SpecularSrvHeapIndex = mTextures["default_specular"]->second;
		}

		if (!item.second.height_tex_path.empty())
		{
			std::string heightName = ExtractFileName(item.second.height_tex_path);
			heightName = heightName.substr(0, heightName.find_last_of("."));
			mat->HeightSrvHeapIndex = mTextures[heightName]->second;
		}
		else
		{
			mat->HeightSrvHeapIndex = mTextures["default_height"]->second;
		}

		if (!item.second.opacity_tex_path.empty())
		{
			std::string opacityName = ExtractFileName(item.second.opacity_tex_path);
			opacityName = opacityName.substr(0, opacityName.find_last_of("."));
			mat->OpacitySrvHeapIndex = mTextures[opacityName]->second;
		}
		else
		{
			mat->OpacitySrvHeapIndex = mTextures["default_opacity"]->second;
		}

		if (!item.second.occlusion_tex_path.empty())
		{
			std::string occlusionName = ExtractFileName(item.second.occlusion_tex_path);
			occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
			mat->OcclusionSrvHeapIndex = mTextures[occlusionName]->second;
		}
		else
		{
			mat->OcclusionSrvHeapIndex = mTextures["default_occlusion"]->second;
		}

		if (!item.second.refraction_tex_path.empty())
		{
			std::string refractionName = ExtractFileName(item.second.refraction_tex_path);
			refractionName = refractionName.substr(0, refractionName.find_last_of("."));
			mat->RefractionSrvHeapIndex = mTextures[refractionName]->second;
		}
		else
		{
			mat->RefractionSrvHeapIndex = mTextures["default_refraction"]->second;
		}

		if (!item.second.emissive_tex_path.empty())
		{
			std::string emssiveName = ExtractFileName(item.second.emissive_tex_path);
			emssiveName = emssiveName.substr(0, emssiveName.find_last_of("."));
			mat->EmissiveSrvHeapIndex = mTextures[emssiveName]->second;
		}
		else
		{
			mat->EmissiveSrvHeapIndex = mTextures["default_emissive"]->second;
		}

		if (!item.second.subsurface_tex_path.empty())
		{
			std::string subsurfaceName = ExtractFileName(item.second.subsurface_tex_path);
			subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
			mat->SubsurfaceSrvHeapIndex = mTextures[subsurfaceName]->second;
		}
		else
		{
			mat->SubsurfaceSrvHeapIndex = mTextures["default_subsurface"]->second;
		}

		if (!item.second.reflection_tex_path.empty())
		{
			std::string reflectionName = ExtractFileName(item.second.reflection_tex_path);
			reflectionName = reflectionName.substr(0, reflectionName.find_last_of("."));
			mat->ReflectionSrvHeapIndex = mTextures[reflectionName]->second;
		}
		else
		{
			mat->ReflectionSrvHeapIndex = mTextures["default_reflection"]->second;
		}

		if (item.second.tint.size() != 0)
		{
			mat->Color = DirectX::XMFLOAT4(item.second.tint[0], item.second.tint[1], item.second.tint[2], item.second.tint[3]);
		}
		else
		{
			mat->Color = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		}

		if (item.second.ref_coeff != 0.04)
		{
			mat->Reflectance = item.second.ref_coeff;
		}
		else
		{
			mat->Reflectance = 0.04f;
		}

		if (item.second.rough_coeff != 0.5)
		{
			mat->Roughness = item.second.rough_coeff;
		}
		else
		{
			mat->Roughness = 0.5f;
		}

		if (item.second.metalness_coeff != 0.0)
		{
			mat->Metalness = item.second.metalness_coeff;
		}
		else
		{
			mat->Metalness = 0.0f;
		}

		mat->Lit = item.second.lit;

		mMaterials[mat->Name] = std::move(mat);
	}
}
