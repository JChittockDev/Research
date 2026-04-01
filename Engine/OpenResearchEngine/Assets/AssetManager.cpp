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

void AssetManager::Build()
{
    SerializeLevel();
    PushLights();
    SetRootSignatures();
    CompileShaders();
    SetPipelineStates();
    ImportTextures();
    PushGenericMesh();
    PushMesh();
    PushMaterials();
    PushRenderItems();

    mOnnxModelResource = std::make_unique<OnnxModelResource>(
        mDevice, mCmdList,
        mGemmRootSignature, mReluRootSignature,
        mLeakyReluRootSignature, mSigmoidRootSignature,
        mTanhRootSignature, mSoftmaxRootSignature,
        mPSOs["Gemm"], mPSOs["Relu"], mPSOs["LeakyRelu"],
        mPSOs["Sigmoid"], mPSOs["Tanh"], mPSOs["Softmax"]);

    mOnnxModelResource->Initialize(GetFullPath("Onnx/test.onnx"));
}

std::string AssetManager::ExtractFileName(const std::string& filePath) {
    size_t found = filePath.find_last_of("/\\");
    return (found != std::string::npos) ? filePath.substr(found + 1) : filePath;
}

// ── SerializeLevel ──────────────────────────────────────────────────────────

void AssetManager::SerializeLevel()
{
    LevelReader levelData(GetFullPath("Levels/DemoLevel.json"));
    mLevelRenderItems["DemoLevel"] = *levelData.level->data->renderItemData->renderItemDataDict;
    mLevelMaterials["DemoLevel"]   = *levelData.level->materialData->pbrMaterialDataDict;
    mLevelLights["DemoLevel"]      = *levelData.level->lightData->lightDataDict;
}

// ── PushLights ──────────────────────────────────────────────────────────────

void AssetManager::PushLights()
{
    for (const auto& item : mLevelLights.at("DemoLevel"))
    {
        Light light;
        light.Position       = DirectX::XMFLOAT3((float)item.second.position[0],  (float)item.second.position[1],  (float)item.second.position[2]);
        light.Direction      = DirectX::XMFLOAT3((float)item.second.direction[0], (float)item.second.direction[1], (float)item.second.direction[2]);
        light.Strength       = DirectX::XMFLOAT3((float)item.second.strength[0],  (float)item.second.strength[1],  (float)item.second.strength[2]);
        light.FalloffStart   = (float)item.second.falloff_start;
        light.FalloffEnd     = (float)item.second.falloff_end;
        light.InnerConeAngle = (float)item.second.inner_cone_angle;
        light.OuterConeAngle = (float)item.second.outer_cone_angle;

        if (item.second.type == "Directional")
        {
            light.Type = 0;
            dynamicLights.DirectionalLights.push_back(light);
        }
        else if (item.second.type == "Point")
        {
            light.Type = 1;
            dynamicLights.PointLights.push_back(light);
        }
        else if (item.second.type == "Spot")
        {
            light.Type = 2;
            dynamicLights.SpotLights.push_back(light);
        }
    }

    dynamicLights.LightTransforms.resize(dynamicLights.GetNumLights());
}

// ── ImportTextures ───────────────────────────────────────────────────────────

void AssetManager::ImportTextures()
{
    // Default Materials will init texture data arrays
    mTextureData = {
        {"default_diffuse",    GetFullPath("Textures/default_diffuse.dds")},
        {"default_nmap",       GetFullPath("Textures/default_nmap.dds")},
        {"default_roughness",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_metalness",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_specular",   GetFullPath("Textures/default_diffuse.dds")},
        {"default_height",     GetFullPath("Textures/default_diffuse.dds")},
        {"default_opacity",    GetFullPath("Textures/default_diffuse.dds")},
        {"default_occlusion",  GetFullPath("Textures/default_diffuse.dds")},
        {"default_refraction", GetFullPath("Textures/default_diffuse.dds")},
        {"default_emissive",   GetFullPath("Textures/default_diffuse.dds")},
        {"default_subsurface", GetFullPath("Textures/default_diffuse.dds")},
        {"default_reflection", GetFullPath("Textures/desertcube1024.dds")}
    };

    for (const auto& item : mLevelMaterials.at("DemoLevel"))
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

// ── PushGenericMesh ──────────────────────────────────────────────────────────

void AssetManager::PushGenericMesh()
{
    // Build "shapeGeo" — procedural shapes referenced by DemoLevel.json.
    // Subsets: "sphere" (sky dome, mirror, shader dome) and "grid" (floor).
    std::string key = GetFullPath("shapeGeo");
    if (mRenderMeshAssets.count(key)) return;

    GeometryGenerator geoGen;
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
    GeometryGenerator::MeshData grid   = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);

    // Vertex layout must match mInputLayout:
    // POSITION float3 | NORMAL float3 | TEXCOORD float2 | TANGENT float4
    struct GeoVertex {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TexC;
        DirectX::XMFLOAT4 Tangent;
    };
    static_assert(sizeof(GeoVertex) == 48, "GeoVertex size mismatch");

    UINT sphereVertexOffset = 0;
    UINT sphereIndexOffset  = 0;
    UINT gridVertexOffset   = (UINT)sphere.Vertices.size();
    UINT gridIndexOffset    = (UINT)sphere.Indices32.size();

    std::vector<GeoVertex> vertices;
    vertices.reserve(sphere.Vertices.size() + grid.Vertices.size());

    auto packVert = [](const GeometryGenerator::Vertex& v) -> GeoVertex {
        return { v.Position, v.Normal, v.TexC, v.TangentU };
    };
    for (auto& v : sphere.Vertices) vertices.push_back(packVert(v));
    for (auto& v : grid.Vertices)   vertices.push_back(packVert(v));

    std::vector<uint32_t> indices;
    indices.reserve(sphere.Indices32.size() + grid.Indices32.size());
    indices.insert(indices.end(), sphere.Indices32.begin(), sphere.Indices32.end());
    indices.insert(indices.end(), grid.Indices32.begin(),   grid.Indices32.end());

    const UINT vbByteSize = (UINT)(vertices.size() * sizeof(GeoVertex));
    const UINT ibByteSize = (UINT)(indices.size()  * sizeof(uint32_t));

    auto asset = std::make_shared<RenderMeshAsset>();
    asset->VertexByteStride     = sizeof(GeoVertex);
    asset->VertexBufferByteSize = vbByteSize;
    asset->IndexFormat          = DXGI_FORMAT_R32_UINT;
    asset->IndexBufferByteSize  = ibByteSize;

    asset->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        mDevice.Get(), mCmdList.Get(),
        vertices.data(), vbByteSize,
        asset->VertexBufferUploader);

    asset->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        mDevice.Get(), mCmdList.Get(),
        indices.data(), ibByteSize,
        asset->IndexBufferUploader);

    SubmeshGeometry sphereSub;
    sphereSub.Name        = "sphere";
    sphereSub.VertexCount = (UINT)sphere.Vertices.size();
    sphereSub.IndexCount  = (UINT)sphere.Indices32.size();
    sphereSub.IndexStart  = sphereIndexOffset;
    sphereSub.VertexStart = sphereVertexOffset;

    SubmeshGeometry gridSub;
    gridSub.Name        = "grid";
    gridSub.VertexCount = (UINT)grid.Vertices.size();
    gridSub.IndexCount  = (UINT)grid.Indices32.size();
    gridSub.IndexStart  = gridIndexOffset;
    gridSub.VertexStart = gridVertexOffset;

    asset->DrawArgs["sphere"] = sphereSub;
    asset->DrawArgs["grid"]   = gridSub;

    mRenderMeshAssets[key] = std::move(asset);
}

// ── PushMesh ─────────────────────────────────────────────────────────────────

void AssetManager::PushMesh()
{
    // ── Light source geometry (static, no animation) ──
    for (const std::string relPath : { "Models/directionallight.obj", "Models/spotlight.obj" }) {
        std::string fullPath = GetFullPath(relPath.c_str());
        if (mRenderMeshAssets.count(fullPath)) continue;
        std::string noVC;
        mRenderMeshAssets[fullPath] = RenderMeshAsset::Load(
            fullPath, noVC, /*forAnimation=*/false,
            mDevice, mCmdList, mSubsets);
    }

    // ── Level items ──
    for (const auto& item : mLevelRenderItems.at("DemoLevel"))
    {
        const ItemData& data = item.second;
        if (data.geometry.empty()) continue;
        bool geoExists = PathExists(data.geometry);
        if (!geoExists) continue;

        std::string fullPath = GetFullPath(data.geometry.c_str());
        if (mRenderMeshAssets.count(fullPath)) continue;

        std::string vcPath;
        std::string texPath = ChangeExtension(fullPath, ".jpg");
        if (PathExists(texPath)) vcPath = texPath;

        mRenderMeshAssets[fullPath] = RenderMeshAsset::Load(
            fullPath, vcPath, data.animated,
            mDevice, mCmdList, mSubsets);

        if (data.simulated) {
            mSimMeshAssets[fullPath] = SimMeshAsset::Load(
                fullPath, vcPath,
                mDevice, mCmdList,
                mRenderMeshAssets[fullPath]->DrawArgs);
        }
    }
}

// ── PushMaterials ────────────────────────────────────────────────────────────

void AssetManager::PushMaterials()
{
    UINT matIndex = 0;

    auto def = std::make_unique<Material>();
    def->Name = "default";
    def->MaterialIndex = matIndex++;
    def->DiffuseSrvHeapIndex    = mTextures["default_diffuse"]->second;
    def->NormalSrvHeapIndex     = mTextures["default_nmap"]->second;
    def->RoughnessSrvHeapIndex  = mTextures["default_roughness"]->second;
    def->MetalnessSrvHeapIndex  = mTextures["default_metalness"]->second;
    def->SpecularSrvHeapIndex   = mTextures["default_specular"]->second;
    def->HeightSrvHeapIndex     = mTextures["default_height"]->second;
    def->OpacitySrvHeapIndex    = mTextures["default_opacity"]->second;
    def->OcclusionSrvHeapIndex  = mTextures["default_occlusion"]->second;
    def->RefractionSrvHeapIndex = mTextures["default_refraction"]->second;
    def->EmissiveSrvHeapIndex   = mTextures["default_emissive"]->second;
    def->SubsurfaceSrvHeapIndex = mTextures["default_subsurface"]->second;
    def->ReflectionSrvHeapIndex = mTextures["default_reflection"]->second;
    def->Color       = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    def->Reflectance = 0.04f;
    def->Roughness   = 0.5f;
    def->Metalness   = 0.0f;
    mMaterials["default"] = std::move(def);

    for (const auto& item : mLevelMaterials.at("DemoLevel"))
    {
        auto mat = std::make_unique<Material>();
        mat->Name          = item.first;
        mat->MaterialIndex = matIndex++;

        if (!item.second.diffuse_tex_path.empty())
        {
            std::string diffuseName = ExtractFileName(item.second.diffuse_tex_path);
            diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
            mat->DiffuseSrvHeapIndex = mTextures[diffuseName]->second;
        }
        else { mat->DiffuseSrvHeapIndex = mTextures["default_diffuse"]->second; }

        if (!item.second.normal_tex_path.empty())
        {
            std::string normalName = ExtractFileName(item.second.normal_tex_path);
            normalName = normalName.substr(0, normalName.find_last_of("."));
            mat->NormalSrvHeapIndex = mTextures[normalName]->second;
        }
        else { mat->NormalSrvHeapIndex = mTextures["default_nmap"]->second; }

        if (!item.second.roughness_tex_path.empty())
        {
            std::string roughnessName = ExtractFileName(item.second.roughness_tex_path);
            roughnessName = roughnessName.substr(0, roughnessName.find_last_of("."));
            mat->RoughnessSrvHeapIndex = mTextures[roughnessName]->second;
        }
        else { mat->RoughnessSrvHeapIndex = mTextures["default_roughness"]->second; }

        if (!item.second.metalness_tex_path.empty())
        {
            std::string metalnessName = ExtractFileName(item.second.metalness_tex_path);
            metalnessName = metalnessName.substr(0, metalnessName.find_last_of("."));
            mat->MetalnessSrvHeapIndex = mTextures[metalnessName]->second;
        }
        else { mat->MetalnessSrvHeapIndex = mTextures["default_metalness"]->second; }

        if (!item.second.specular_tex_path.empty())
        {
            std::string specularName = ExtractFileName(item.second.specular_tex_path);
            specularName = specularName.substr(0, specularName.find_last_of("."));
            mat->SpecularSrvHeapIndex = mTextures[specularName]->second;
        }
        else { mat->SpecularSrvHeapIndex = mTextures["default_specular"]->second; }

        if (!item.second.height_tex_path.empty())
        {
            std::string heightName = ExtractFileName(item.second.height_tex_path);
            heightName = heightName.substr(0, heightName.find_last_of("."));
            mat->HeightSrvHeapIndex = mTextures[heightName]->second;
        }
        else { mat->HeightSrvHeapIndex = mTextures["default_height"]->second; }

        if (!item.second.opacity_tex_path.empty())
        {
            std::string opacityName = ExtractFileName(item.second.opacity_tex_path);
            opacityName = opacityName.substr(0, opacityName.find_last_of("."));
            mat->OpacitySrvHeapIndex = mTextures[opacityName]->second;
        }
        else { mat->OpacitySrvHeapIndex = mTextures["default_opacity"]->second; }

        if (!item.second.occlusion_tex_path.empty())
        {
            std::string occlusionName = ExtractFileName(item.second.occlusion_tex_path);
            occlusionName = occlusionName.substr(0, occlusionName.find_last_of("."));
            mat->OcclusionSrvHeapIndex = mTextures[occlusionName]->second;
        }
        else { mat->OcclusionSrvHeapIndex = mTextures["default_occlusion"]->second; }

        if (!item.second.refraction_tex_path.empty())
        {
            std::string refractionName = ExtractFileName(item.second.refraction_tex_path);
            refractionName = refractionName.substr(0, refractionName.find_last_of("."));
            mat->RefractionSrvHeapIndex = mTextures[refractionName]->second;
        }
        else { mat->RefractionSrvHeapIndex = mTextures["default_refraction"]->second; }

        if (!item.second.emissive_tex_path.empty())
        {
            std::string emssiveName = ExtractFileName(item.second.emissive_tex_path);
            emssiveName = emssiveName.substr(0, emssiveName.find_last_of("."));
            mat->EmissiveSrvHeapIndex = mTextures[emssiveName]->second;
        }
        else { mat->EmissiveSrvHeapIndex = mTextures["default_emissive"]->second; }

        if (!item.second.subsurface_tex_path.empty())
        {
            std::string subsurfaceName = ExtractFileName(item.second.subsurface_tex_path);
            subsurfaceName = subsurfaceName.substr(0, subsurfaceName.find_last_of("."));
            mat->SubsurfaceSrvHeapIndex = mTextures[subsurfaceName]->second;
        }
        else { mat->SubsurfaceSrvHeapIndex = mTextures["default_subsurface"]->second; }

        if (!item.second.reflection_tex_path.empty())
        {
            std::string reflectionName = ExtractFileName(item.second.reflection_tex_path);
            reflectionName = reflectionName.substr(0, reflectionName.find_last_of("."));
            mat->ReflectionSrvHeapIndex = mTextures[reflectionName]->second;
        }
        else { mat->ReflectionSrvHeapIndex = mTextures["default_reflection"]->second; }

        mat->Color       = (item.second.tint.size() != 0)
            ? DirectX::XMFLOAT4(item.second.tint[0], item.second.tint[1], item.second.tint[2], item.second.tint[3])
            : DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        mat->Reflectance = (item.second.ref_coeff  != 0.04) ? item.second.ref_coeff  : 0.04f;
        mat->Roughness   = (item.second.rough_coeff != 0.5) ? item.second.rough_coeff : 0.5f;
        mat->Metalness   = (item.second.metalness_coeff != 0.0) ? item.second.metalness_coeff : 0.0f;
        mat->Lit         = item.second.lit;

        mMaterials[mat->Name] = std::move(mat);
    }
}

// ── PushRenderItems ──────────────────────────────────────────────────────────

void AssetManager::PushRenderItems()
{
    auto BuildAndAssign = [&](
        const std::string&  fullPath,
        const ItemData*     data,
        RenderMeshAsset*    asset,
        std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& riMap)
    {
        if (mMeshInstances.count(fullPath)) return;

        DeformationGraph graph;

        if (data && data->animated && asset->hasBlendshapes)
        {
            UINT bsCBIdx = (UINT)mBlendshapeDeformers.size();
            auto bsCtrl  = mBlendshapeControllers.count(fullPath)
                ? mBlendshapeControllers.at(fullPath) : nullptr;
            auto bs = std::make_unique<BlendshapeDeformer>(
                mDevice.Get(), mCmdList.Get(),
                asset, bsCBIdx, bsCtrl,
                mBlendRootSignature.Get(), mPSOs.at("blend").Get());
            mBlendshapeDeformers.push_back(bs.get());
            graph.AddDeformer(std::move(bs));
        }

        if (data && data->animated && asset->hasSkinning)
        {
            UINT skinCBIdx = (UINT)mSkinDeformers.size();
            auto skinCtrl  = mSkinningControllers.count(fullPath)
                ? mSkinningControllers.at(fullPath) : nullptr;
            auto sk = std::make_unique<SkinDeformer>(
                mDevice.Get(), mCmdList.Get(),
                asset, skinCBIdx, skinCtrl,
                mSkinnedRootSignature.Get(), mPSOs.at("skinned").Get());
            mSkinDeformers.push_back(sk.get());
            graph.AddDeformer(std::move(sk));
        }

        if (data && data->simulated && mSimMeshAssets.count(fullPath))
        {
            PhysicsDeformerResources res{};
            MakePhysicsDeformerResources(res);
            auto phys = std::make_unique<PhysicsDeformer>(
                mDevice.Get(), mCmdList.Get(),
                mSimMeshAssets.at(fullPath).get(),
                res);
            graph.AddDeformer(std::move(phys));
        }

        auto instance = std::make_shared<MeshInstance>(asset, std::move(graph));
        mMeshInstances[fullPath] = instance;

        for (auto& [key, vec] : riMap)
        {
            for (auto& ri : vec)
            {
                if (ri->Instance) continue;
                if (asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = instance.get();
            }
        }
    };

    // ── Directional light render items ──
    {
        const std::string relPath  = "Models/directionallight.obj";
        const std::string fullPath = GetFullPath(relPath.c_str());

        if (mRenderMeshAssets.count(fullPath))
        {
            RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
            UINT lightOffset = 0;
            for (int i = 0; i < (int)dynamicLights.DirectionalLights.size(); i++)
            {
                for (auto& [subsetName, args] : asset->DrawArgs)
                {
                    auto ritem = std::make_shared<RenderItem>();
                    ritem->World         = Math::Identity4x4();
                    ritem->TexTransform  = Math::Identity4x4();
                    ritem->ObjCBIndex    = ObjectCBIndex++;
                    ritem->Mat           = mMaterials.at("default").get();
                    ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    ritem->SubsetName    = subsetName;

                    mRenderItemLayers["Opaque"].push_back(ritem);
                    mRenderItems.push_back(ritem);
                    std::string key = subsetName + "_" + std::to_string(lightOffset++);
                    mDirectionalLightRenderItemMap[key].push_back(ritem);
                }
            }

            BuildAndAssign(fullPath, nullptr, asset, mDirectionalLightRenderItemMap);
            for (auto& ri : mRenderItemLayers["Opaque"])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = mMeshInstances.at(fullPath).get();
            }
        }
    }

    // ── Spot light render items ──
    {
        const std::string relPath  = "Models/spotlight.obj";
        const std::string fullPath = GetFullPath(relPath.c_str());

        if (mRenderMeshAssets.count(fullPath))
        {
            RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
            UINT lightOffset = 0;
            for (int i = 0; i < (int)dynamicLights.SpotLights.size(); i++)
            {
                for (auto& [subsetName, args] : asset->DrawArgs)
                {
                    auto ritem = std::make_shared<RenderItem>();
                    ritem->World         = Math::Identity4x4();
                    ritem->TexTransform  = Math::Identity4x4();
                    ritem->ObjCBIndex    = ObjectCBIndex++;
                    ritem->Mat           = mMaterials.at("default").get();
                    ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    ritem->SubsetName    = subsetName;

                    mRenderItemLayers["Opaque"].push_back(ritem);
                    mRenderItems.push_back(ritem);
                    std::string key = subsetName + "_" + std::to_string(lightOffset++);
                    mSpotLightRenderItemMap[key].push_back(ritem);
                }
            }

            BuildAndAssign(fullPath, nullptr, asset, mSpotLightRenderItemMap);
            for (auto& ri : mRenderItemLayers["Opaque"])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = mMeshInstances.at(fullPath).get();
            }
        }
    }

    // ── Level render items ──
    for (const auto& item : mLevelRenderItems.at("DemoLevel"))
    {
        const std::string& itemName        = item.first;
        const ItemData&    renderItemData  = item.second;

        if (renderItemData.geometry.empty()) continue;
        bool geoExists = PathExists(renderItemData.geometry);
        if (!geoExists) continue;

        std::string fullPath = GetFullPath(renderItemData.geometry.c_str());
        if (!mRenderMeshAssets.count(fullPath)) continue;

        RenderMeshAsset* asset = mRenderMeshAssets.at(fullPath).get();
        DirectX::XMFLOAT4X4 transformMatrix = Math::CreateTransformMatrix(
            DirectX::XMFLOAT3((float)renderItemData.position[0], (float)renderItemData.position[1], (float)renderItemData.position[2]),
            DirectX::XMFLOAT4((float)renderItemData.rotation[0], (float)renderItemData.rotation[1], (float)renderItemData.rotation[2], (float)renderItemData.rotation[3]),
            DirectX::XMFLOAT3((float)renderItemData.scale[0],    (float)renderItemData.scale[1],    (float)renderItemData.scale[2]));

        const auto& settings = renderItemData.settings;

        for (auto& [subsetName, args] : asset->DrawArgs)
        {
            if (settings.find(subsetName) == settings.end()) continue;

            auto ritem = std::make_shared<RenderItem>();
            ritem->World         = transformMatrix;
            ritem->TexTransform  = Math::Identity4x4();
            ritem->ObjCBIndex    = ObjectCBIndex++;
            ritem->Mat           = mMaterials.at(settings.at(subsetName).Material).get();
            ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            ritem->SubsetName    = subsetName;

            mRenderItemLayers[renderItemData.render_layer].push_back(ritem);
            mRenderItems.push_back(ritem);
            mMeshRenderItemMap[itemName].push_back(ritem);
        }

        BuildAndAssign(fullPath, &renderItemData, asset, mMeshRenderItemMap);

        if (mMeshInstances.count(fullPath))
        {
            auto& inst = mMeshInstances.at(fullPath);
            for (auto& ri : mRenderItemLayers[renderItemData.render_layer])
            {
                if (!ri->Instance && asset->DrawArgs.count(ri->SubsetName))
                    ri->Instance = inst.get();
            }
        }
    }
}

// ── MakePhysicsDeformerResources ─────────────────────────────────────────────

void AssetManager::MakePhysicsDeformerResources(PhysicsDeformerResources& res)
{
    res.meshTransfer       = mMeshTransferRootSignature.Get();
    res.psoMeshTransfer    = mPSOs.at("meshTransfer").Get();
    res.tension            = mTensionRootSignature.Get();
    res.psoTension         = mPSOs.at("tension").Get();
    res.force              = mForceRootSignature.Get();
    res.psoForce           = mPSOs.at("force").Get();
    res.preSolve           = mPreSolveRootSignature.Get();
    res.psoPreSolve        = mPSOs.at("preSolve").Get();
    res.constraintSolve    = mConstraintSolveRootSignature.Get();
    res.psoConstraintSolve = mPSOs.at("constraintSolve").Get();
    res.postSolve          = mPostSolveRootSignature.Get();
    res.psoPostSolve       = mPSOs.at("postSolve").Get();
    res.simMeshTransfer    = mSimMeshTransferRootSignature.Get();
    res.psoSimMeshTransfer = mPSOs.at("simMeshTransfer").Get();
    res.triangleNormal     = mTriangleNormalRootSignature.Get();
    res.psoTriangleNormal  = mPSOs.at("triangleNormal").Get();
    res.vertexNormal       = mVertexNormalRootSignature.Get();
    res.psoVertexNormal    = mPSOs.at("vertexNormal").Get();
}

// ── SetRootSignatures ─────────────────────────────────────────────────────────

void AssetManager::SetRootSignatures()
{
    SetGenericRootSignature();
    SetGBufferRootSignature();
    SetSsaoRootSignature();
    SetSsgiRootSignature();
    SetSssRootSignature();
    SetPoissonBlurRootSignature();
    SetEdgeBlurRootSignature();
    SetColorEdgeBlurRootSignature();
    SetCompositeRootSignature();
    SetShadowsRootSignature();
    SetLightingRootSignature();
    SetBlendRootSignature();
    SetSkinnedRootSignature();
    SetMeshTransferRootSignature();
    SetTensionRootSignature();
    SetForceRootSignature();
    SetPreSolveRootSignature();
    SetConstraintSolveRootSignature();
    SetPostSolveRootSignature();
    SetSimMeshTransferRootSignature();
    SetTriangleNormalRootSignature();
    SetVertexNormalRootSignature();
    SetRadianceRootSignature();
    SetGemmRootSignature();
    SetReluRootSignature();
    SetLeakyReluRootSignature();
    SetTanhRootSignature();
    SetSigmoidRootSignature();
    SetSoftmaxRootSignature();
}

void AssetManager::SetGenericRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 18, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 18, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[5];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void AssetManager::SetGBufferRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 0, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mGBufferRootSignature.GetAddressOf())));
}

void AssetManager::SetSsaoRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[3];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSsaoRootSignature.GetAddressOf())));
}

void AssetManager::SetEdgeBlurRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mEdgeBlurRootSignature.GetAddressOf())));
}

void AssetManager::SetPoissonBlurRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[3];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mPoissonBlurRootSignature.GetAddressOf())));
}

void AssetManager::SetColorEdgeBlurRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mColorEdgeBlurRootSignature.GetAddressOf())));
}

void AssetManager::SetSsgiRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 2, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSsgiRootSignature.GetAddressOf())));
}

void AssetManager::SetSssRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable3;
    texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable4;
    texTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable5;
    texTable5.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 6, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[8];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsShaderResourceView(0, 1);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[5].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[6].InitAsDescriptorTable(1, &texTable4, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[7].InitAsDescriptorTable(1, &texTable5, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSssRootSignature.GetAddressOf())));
}

void AssetManager::SetShadowsRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 0, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mShadowsRootSignature.GetAddressOf())));
}

void AssetManager::SetLightingRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 11, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[6];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mLightingRootSignature.GetAddressOf())));
}

void AssetManager::SetRadianceRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[5];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
    slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mRadianceRootSignature.GetAddressOf())));
}

void AssetManager::SetCompositeRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable0;
    texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[3];
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mCompositeRootSignature.GetAddressOf())));
}

void AssetManager::SetBlendRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[4];
    rootParameters[0].InitAsConstants(1, 0);
    rootParameters[1].InitAsConstantBufferView(1);
    rootParameters[2].InitAsShaderResourceView(0);
    rootParameters[3].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mBlendRootSignature.GetAddressOf())));
}

void AssetManager::SetSkinnedRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[4];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsShaderResourceView(1);
    rootParameters[3].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSkinnedRootSignature.GetAddressOf())));
}

void AssetManager::SetTriangleNormalRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mTriangleNormalRootSignature.GetAddressOf())));
}

void AssetManager::SetVertexNormalRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[4];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsShaderResourceView(2);
    rootParameters[3].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mVertexNormalRootSignature.GetAddressOf())));
}

void AssetManager::SetMeshTransferRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mMeshTransferRootSignature.GetAddressOf())));
}

void AssetManager::SetSimMeshTransferRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSimMeshTransferRootSignature.GetAddressOf())));
}

void AssetManager::SetForceRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsUnorderedAccessView(0);
    rootParameters[2].InitAsUnorderedAccessView(1);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mForceRootSignature.GetAddressOf())));
}

void AssetManager::SetPreSolveRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mPreSolveRootSignature.GetAddressOf())));
}

void AssetManager::SetConstraintSolveRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[5];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsShaderResourceView(2);
    rootParameters[3].InitAsUnorderedAccessView(0);
    rootParameters[4].InitAsUnorderedAccessView(1);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(5, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mConstraintSolveRootSignature.GetAddressOf())));
}

void AssetManager::SetPostSolveRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[8];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsShaderResourceView(2);
    rootParameters[3].InitAsShaderResourceView(3);
    rootParameters[4].InitAsShaderResourceView(4);
    rootParameters[5].InitAsShaderResourceView(5);
    rootParameters[6].InitAsShaderResourceView(6);
    rootParameters[7].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(8, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mPostSolveRootSignature.GetAddressOf())));
}

void AssetManager::SetTensionRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsShaderResourceView(0);
    rootParameters[1].InitAsShaderResourceView(1);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mTensionRootSignature.GetAddressOf())));
}

void AssetManager::SetGemmRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[5];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsShaderResourceView(1);
    rootParameters[3].InitAsShaderResourceView(2);
    rootParameters[4].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(5, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mGemmRootSignature.GetAddressOf())));
}

void AssetManager::SetReluRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mReluRootSignature.GetAddressOf())));
}

void AssetManager::SetLeakyReluRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mLeakyReluRootSignature.GetAddressOf())));
}

void AssetManager::SetSigmoidRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSigmoidRootSignature.GetAddressOf())));
}

void AssetManager::SetSoftmaxRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mSoftmaxRootSignature.GetAddressOf())));
}

void AssetManager::SetTanhRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(4, 0);
    rootParameters[1].InitAsShaderResourceView(0);
    rootParameters[2].InitAsUnorderedAccessView(0);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParameters, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    if (errorBlob != nullptr) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(hr);
    ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(mTanhRootSignature.GetAddressOf())));
}

// ── CompileShaders ────────────────────────────────────────────────────────────

void AssetManager::CompileShaders()
{
    std::string numDirectionalLights = std::to_string(dynamicLights.DirectionalLights.size());
    std::string numPointLights       = std::to_string(dynamicLights.PointLights.size());
    std::string numSpotLights        = std::to_string(dynamicLights.SpotLights.size());

    D3D_SHADER_MACRO directionalLightMacro = { "NUM_DIR_LIGHTS",   numDirectionalLights.c_str() };
    D3D_SHADER_MACRO pointLightMacro       = { "NUM_POINT_LIGHTS", numPointLights.c_str() };
    D3D_SHADER_MACRO spotLightMacro        = { "NUM_SPOT_LIGHTS",  numSpotLights.c_str() };
    D3D_SHADER_MACRO skinningMacro         = { "SKINNED",          "1" };
    D3D_SHADER_MACRO alphaMacro            = { "ALPHA_TEST",       "1" };
    std::string factor = std::to_string(32768.0);
    D3D_SHADER_MACRO quantizeMacro         = { "QUANTIZE",         factor.c_str() };
    D3D_SHADER_MACRO null                  = { NULL, NULL };

    const D3D_SHADER_MACRO opaqueDefines[4]    = { directionalLightMacro, pointLightMacro, spotLightMacro, null };
    const D3D_SHADER_MACRO alphaTestDefines[2] = { alphaMacro, null };
    const D3D_SHADER_MACRO skinnedDefines[2]   = { skinningMacro, null };
    const D3D_SHADER_MACRO clothDefines[2]     = { quantizeMacro, null };

    auto CompileW = [&](const char* relPath, const D3D_SHADER_MACRO* defines, const char* entry, const char* target) {
        std::string p = GetFullPath(relPath);
        std::wstring wp(p.begin(), p.end());
        return d3dUtil::CompileShader(wp, defines, entry, target);
    };

    mShaders["LightingVS"]  = CompileW("Shaders/Lighting.hlsl",  nullptr,       "VS", "vs_5_1");
    mShaders["LightingPS"]  = CompileW("Shaders/Lighting.hlsl",  opaqueDefines, "PS", "ps_5_1");

    mShaders["RadianceVS"]  = CompileW("Shaders/Radiance.hlsl",  nullptr,       "VS", "vs_5_1");
    mShaders["RadiancePS"]  = CompileW("Shaders/Radiance.hlsl",  opaqueDefines, "PS", "ps_5_1");

    mShaders["SssVS"]       = CompileW("Shaders/Sss.hlsl",       nullptr,       "VS", "vs_5_1");
    mShaders["SssPS"]       = CompileW("Shaders/Sss.hlsl",       opaqueDefines, "PS", "ps_5_1");

    mShaders["SsaoVS"]      = CompileW("Shaders/Ssao.hlsl",      nullptr,       "VS", "vs_5_1");
    mShaders["SsaoPS"]      = CompileW("Shaders/Ssao.hlsl",      opaqueDefines, "PS", "ps_5_1");

    mShaders["SsgiVS"]      = CompileW("Shaders/Ssgi.hlsl",      nullptr,       "VS", "vs_5_1");
    mShaders["SsgiPS"]      = CompileW("Shaders/Ssgi.hlsl",      opaqueDefines, "PS", "ps_5_1");

    mShaders["EdgeBlurVS"]  = CompileW("Shaders/EdgeBlur.hlsl",  nullptr,       "VS", "vs_5_1");
    mShaders["EdgeBlurPS"]  = CompileW("Shaders/EdgeBlur.hlsl",  opaqueDefines, "PS", "ps_5_1");

    mShaders["PoissonBlurVS"] = CompileW("Shaders/PoissonBlur.hlsl", nullptr,       "VS", "vs_5_1");
    mShaders["PoissonBlurPS"] = CompileW("Shaders/PoissonBlur.hlsl", opaqueDefines, "PS", "ps_5_1");

    mShaders["GBufferVS"]   = CompileW("Shaders/GBuffer.hlsl",   nullptr,       "VS", "vs_5_1");
    mShaders["GBufferPS"]   = CompileW("Shaders/GBuffer.hlsl",   opaqueDefines, "PS", "ps_5_1");

    mShaders["CompositeVS"] = CompileW("Shaders/Composite.hlsl", nullptr,       "VS", "vs_5_1");
    mShaders["CompositePS"] = CompileW("Shaders/Composite.hlsl", opaqueDefines, "PS", "ps_5_1");

    mShaders["shadowVS"]           = CompileW("Shaders/Shadows.hlsl", nullptr,          "VS", "vs_5_1");
    mShaders["shadowOpaquePS"]     = CompileW("Shaders/Shadows.hlsl", nullptr,          "PS", "ps_5_1");
    mShaders["shadowAlphaTestedPS"]= CompileW("Shaders/Shadows.hlsl", alphaTestDefines, "PS", "ps_5_1");

    mShaders["blendCS"]            = CompileW("Shaders/ComputeBlendshapes.hlsl",   nullptr,      "CS", "cs_5_1");
    mShaders["skinnedCS"]          = CompileW("Shaders/ComputeSkinning.hlsl",       nullptr,      "CS", "cs_5_1");
    mShaders["triangleNormalCS"]   = CompileW("Shaders/ComputeTriangleNormals.hlsl",nullptr,      "CS", "cs_5_1");
    mShaders["vertexNormalCS"]     = CompileW("Shaders/ComputeVertexNormals.hlsl",  nullptr,      "CS", "cs_5_1");
    mShaders["meshTransferCS"]     = CompileW("Shaders/ComputeMeshTransfer.hlsl",   nullptr,      "CS", "cs_5_1");
    mShaders["simMeshTransferCS"]  = CompileW("Shaders/ComputeSimMeshTransfer.hlsl",nullptr,      "CS", "cs_5_1");
    mShaders["tensionCS"]          = CompileW("Shaders/ComputeTension.hlsl",        nullptr,      "CS", "cs_5_1");
    mShaders["forceCS"]            = CompileW("Shaders/ComputeForce.hlsl",          nullptr,      "CS", "cs_5_1");
    mShaders["preSolveCS"]         = CompileW("Shaders/ComputePreSolve.hlsl",       nullptr,      "CS", "cs_5_1");
    mShaders["constraintSolveCS"]  = CompileW("Shaders/ComputeConstraintSolve.hlsl",clothDefines, "CS", "cs_5_1");
    mShaders["postSolveCS"]        = CompileW("Shaders/ComputePostSolve.hlsl",      clothDefines, "CS", "cs_5_1");
    mShaders["GemmCS"]             = CompileW("Shaders/ML/Gemm.hlsl",               nullptr,      "CS", "cs_5_1");
    mShaders["LeakyReluCS"]        = CompileW("Shaders/ML/LeakyReLU.hlsl",          nullptr,      "CS", "cs_5_1");
    mShaders["ReluCS"]             = CompileW("Shaders/ML/ReLU.hlsl",               nullptr,      "CS", "cs_5_1");
    mShaders["SigmoidCS"]          = CompileW("Shaders/ML/Sigmoid.hlsl",            nullptr,      "CS", "cs_5_1");
    mShaders["SoftmaxCS"]          = CompileW("Shaders/ML/Softmax.hlsl",            nullptr,      "CS", "cs_5_1");
    mShaders["TanhCS"]             = CompileW("Shaders/ML/Tanh.hlsl",               nullptr,      "CS", "cs_5_1");

    mInputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };
}

// ── SetPipelineStates ─────────────────────────────────────────────────────────

void AssetManager::SetPipelineStates()
{
    // Compute PSOs
    auto MakeComputePSO = [&](const char* key, ID3D12RootSignature* rootSig) {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = rootSig;
        desc.CS = { reinterpret_cast<BYTE*>(mShaders[key]->GetBufferPointer()), mShaders[key]->GetBufferSize() };
        std::string psoKey = key;
        // Strip the trailing "CS" to get the PSO key
        psoKey = psoKey.substr(0, psoKey.size() - 2);
        ThrowIfFailed(mDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mPSOs[psoKey])));
    };

    MakeComputePSO("blendCS",           mBlendRootSignature.Get());
    MakeComputePSO("skinnedCS",         mSkinnedRootSignature.Get());
    MakeComputePSO("triangleNormalCS",  mTriangleNormalRootSignature.Get());
    MakeComputePSO("vertexNormalCS",    mVertexNormalRootSignature.Get());
    MakeComputePSO("meshTransferCS",    mMeshTransferRootSignature.Get());
    MakeComputePSO("simMeshTransferCS", mSimMeshTransferRootSignature.Get());
    MakeComputePSO("forceCS",           mForceRootSignature.Get());
    MakeComputePSO("tensionCS",         mTensionRootSignature.Get());
    MakeComputePSO("preSolveCS",        mPreSolveRootSignature.Get());
    MakeComputePSO("postSolveCS",       mPostSolveRootSignature.Get());
    MakeComputePSO("constraintSolveCS", mConstraintSolveRootSignature.Get());

    // ML Compute PSOs (key doesn't follow "CS" suffix convention uniformly)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mGemmRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["GemmCS"]->GetBufferPointer()), mShaders["GemmCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["Gemm"])));
    }
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mLeakyReluRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["LeakyReluCS"]->GetBufferPointer()), mShaders["LeakyReluCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["LeakyRelu"])));
    }
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mReluRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["ReluCS"]->GetBufferPointer()), mShaders["ReluCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["Relu"])));
    }
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mSigmoidRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["SigmoidCS"]->GetBufferPointer()), mShaders["SigmoidCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["Sigmoid"])));
    }
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mTanhRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["TanhCS"]->GetBufferPointer()), mShaders["TanhCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["Tanh"])));
    }
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
        d.pRootSignature = mSoftmaxRootSignature.Get();
        d.CS = { reinterpret_cast<BYTE*>(mShaders["SoftmaxCS"]->GetBufferPointer()), mShaders["SoftmaxCS"]->GetBufferSize() };
        ThrowIfFailed(mDevice->CreateComputePipelineState(&d, IID_PPV_ARGS(&mPSOs["Softmax"])));
    }

    // Base graphics PSO desc
    D3D12_RASTERIZER_DESC noCullingRasterDesc = {};
    noCullingRasterDesc.FillMode             = D3D12_FILL_MODE_SOLID;
    noCullingRasterDesc.CullMode             = D3D12_CULL_MODE_NONE;
    noCullingRasterDesc.FrontCounterClockwise= FALSE;
    noCullingRasterDesc.DepthClipEnable      = TRUE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout          = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaquePsoDesc.RasterizerState      = noCullingRasterDesc;
    opaquePsoDesc.BlendState           = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState    = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask           = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets     = 1;
    opaquePsoDesc.RTVFormats[0]        = mBackBufferFormat;
    opaquePsoDesc.SampleDesc.Count     = mMsaaState ? 4 : 1;
    opaquePsoDesc.SampleDesc.Quality   = mMsaaState ? (mMsaaQuality - 1) : 0;
    opaquePsoDesc.DSVFormat            = mDepthStencilFormat;

    // Shadow PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
    smapPsoDesc.RasterizerState.DepthBias           = 100000;
    smapPsoDesc.RasterizerState.DepthBiasClamp      = 0.0f;
    smapPsoDesc.RasterizerState.SlopeScaledDepthBias= 1.0f;
    smapPsoDesc.pRootSignature    = mShadowsRootSignature.Get();
    smapPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["shadowVS"]->GetBufferPointer()), mShaders["shadowVS"]->GetBufferSize() };
    smapPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["shadowOpaquePS"]->GetBufferPointer()), mShaders["shadowOpaquePS"]->GetBufferSize() };
    smapPsoDesc.RTVFormats[0]     = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets  = 0;
    ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_opaque"])));

    // GBuffer PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gBufferPsoDesc = opaquePsoDesc;
    gBufferPsoDesc.pRootSignature = mGBufferRootSignature.Get();
    gBufferPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["GBufferVS"]->GetBufferPointer()), mShaders["GBufferVS"]->GetBufferSize() };
    gBufferPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["GBufferPS"]->GetBufferPointer()), mShaders["GBufferPS"]->GetBufferSize() };
    gBufferPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    gBufferPsoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    gBufferPsoDesc.RTVFormats[2] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    gBufferPsoDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
    gBufferPsoDesc.RTVFormats[4] = DXGI_FORMAT_R8G8B8A8_UNORM;
    gBufferPsoDesc.RTVFormats[5] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gBufferPsoDesc.RTVFormats[6] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    gBufferPsoDesc.NumRenderTargets = 7;
    mDevice->CreateGraphicsPipelineState(&gBufferPsoDesc, IID_PPV_ARGS(&mPSOs["GBuffer"]));

    // Radiance PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gRadiancePsoDesc = opaquePsoDesc;
    gRadiancePsoDesc.pRootSignature = mRadianceRootSignature.Get();
    gRadiancePsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["RadianceVS"]->GetBufferPointer()), mShaders["RadianceVS"]->GetBufferSize() };
    gRadiancePsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["RadiancePS"]->GetBufferPointer()), mShaders["RadiancePS"]->GetBufferSize() };
    gRadiancePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    gRadiancePsoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
    gRadiancePsoDesc.DepthStencilState.DepthEnable    = false;
    gRadiancePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    gRadiancePsoDesc.NumRenderTargets = 2;
    mDevice->CreateGraphicsPipelineState(&gRadiancePsoDesc, IID_PPV_ARGS(&mPSOs["Radiance"]));

    // Lighting PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightingPsoDesc = opaquePsoDesc;
    lightingPsoDesc.InputLayout  = { nullptr, 0 };
    lightingPsoDesc.pRootSignature = mLightingRootSignature.Get();
    lightingPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    lightingPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["LightingVS"]->GetBufferPointer()), mShaders["LightingVS"]->GetBufferSize() };
    lightingPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["LightingPS"]->GetBufferPointer()), mShaders["LightingPS"]->GetBufferSize() };
    lightingPsoDesc.DepthStencilState.DepthEnable    = false;
    lightingPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&lightingPsoDesc, IID_PPV_ARGS(&mPSOs["Lighting"]));

    // SSAO PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = opaquePsoDesc;
    ssaoPsoDesc.InputLayout  = { nullptr, 0 };
    ssaoPsoDesc.pRootSignature = mSsaoRootSignature.Get();
    ssaoPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16_UNORM;
    ssaoPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["SsaoVS"]->GetBufferPointer()), mShaders["SsaoVS"]->GetBufferSize() };
    ssaoPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["SsaoPS"]->GetBufferPointer()), mShaders["SsaoPS"]->GetBufferSize() };
    ssaoPsoDesc.DepthStencilState.DepthEnable    = false;
    ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&mPSOs["Ssao"]));

    // EdgeBlur PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC edgeBlurPsoDesc = opaquePsoDesc;
    edgeBlurPsoDesc.InputLayout  = { nullptr, 0 };
    edgeBlurPsoDesc.pRootSignature = mEdgeBlurRootSignature.Get();
    edgeBlurPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16_UNORM;
    edgeBlurPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["EdgeBlurVS"]->GetBufferPointer()), mShaders["EdgeBlurVS"]->GetBufferSize() };
    edgeBlurPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["EdgeBlurPS"]->GetBufferPointer()), mShaders["EdgeBlurPS"]->GetBufferSize() };
    edgeBlurPsoDesc.DepthStencilState.DepthEnable    = false;
    edgeBlurPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&edgeBlurPsoDesc, IID_PPV_ARGS(&mPSOs["EdgeBlur"]));

    // PoissonBlur PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC poissonBlurPsoDesc = opaquePsoDesc;
    poissonBlurPsoDesc.InputLayout  = { nullptr, 0 };
    poissonBlurPsoDesc.pRootSignature = mPoissonBlurRootSignature.Get();
    poissonBlurPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    poissonBlurPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["PoissonBlurVS"]->GetBufferPointer()), mShaders["PoissonBlurVS"]->GetBufferSize() };
    poissonBlurPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["PoissonBlurPS"]->GetBufferPointer()), mShaders["PoissonBlurPS"]->GetBufferSize() };
    poissonBlurPsoDesc.DepthStencilState.DepthEnable    = false;
    poissonBlurPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&poissonBlurPsoDesc, IID_PPV_ARGS(&mPSOs["PoissonBlur"]));

    // SSGI PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ssgiPsoDesc = opaquePsoDesc;
    ssgiPsoDesc.InputLayout  = { nullptr, 0 };
    ssgiPsoDesc.pRootSignature = mSsgiRootSignature.Get();
    ssgiPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    ssgiPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["SsgiVS"]->GetBufferPointer()), mShaders["SsgiVS"]->GetBufferSize() };
    ssgiPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["SsgiPS"]->GetBufferPointer()), mShaders["SsgiPS"]->GetBufferSize() };
    ssgiPsoDesc.DepthStencilState.DepthEnable    = false;
    ssgiPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&ssgiPsoDesc, IID_PPV_ARGS(&mPSOs["Ssgi"]));

    // ColorEdgeBlur PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC colorEdgeBlurPsoDesc = opaquePsoDesc;
    colorEdgeBlurPsoDesc.InputLayout  = { nullptr, 0 };
    colorEdgeBlurPsoDesc.pRootSignature = mColorEdgeBlurRootSignature.Get();
    colorEdgeBlurPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    colorEdgeBlurPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["EdgeBlurVS"]->GetBufferPointer()), mShaders["EdgeBlurVS"]->GetBufferSize() };
    colorEdgeBlurPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["EdgeBlurPS"]->GetBufferPointer()), mShaders["EdgeBlurPS"]->GetBufferSize() };
    colorEdgeBlurPsoDesc.DepthStencilState.DepthEnable    = false;
    colorEdgeBlurPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&colorEdgeBlurPsoDesc, IID_PPV_ARGS(&mPSOs["ColorEdgeBlur"]));

    // Composite PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC compositePsoDesc = opaquePsoDesc;
    compositePsoDesc.InputLayout  = { nullptr, 0 };
    compositePsoDesc.pRootSignature = mCompositeRootSignature.Get();
    compositePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    compositePsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["CompositeVS"]->GetBufferPointer()), mShaders["CompositeVS"]->GetBufferSize() };
    compositePsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["CompositePS"]->GetBufferPointer()), mShaders["CompositePS"]->GetBufferSize() };
    compositePsoDesc.DepthStencilState.DepthEnable    = false;
    compositePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&compositePsoDesc, IID_PPV_ARGS(&mPSOs["Composite"]));

    // SSS PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC sssPsoDesc = opaquePsoDesc;
    sssPsoDesc.InputLayout  = { nullptr, 0 };
    sssPsoDesc.pRootSignature = mSssRootSignature.Get();
    sssPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    sssPsoDesc.VS = { reinterpret_cast<BYTE*>(mShaders["SssVS"]->GetBufferPointer()), mShaders["SssVS"]->GetBufferSize() };
    sssPsoDesc.PS = { reinterpret_cast<BYTE*>(mShaders["SssPS"]->GetBufferPointer()), mShaders["SssPS"]->GetBufferSize() };
    sssPsoDesc.DepthStencilState.DepthEnable    = false;
    sssPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    mDevice->CreateGraphicsPipelineState(&sssPsoDesc, IID_PPV_ARGS(&mPSOs["Sss"]));
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> AssetManager::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0.0f,
		8);

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	const CD3DX12_STATIC_SAMPLER_DESC depth(
		7,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow, depth
	};
}
