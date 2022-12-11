#include "SkinnedMesh.h"
#include "../Models/External/LoadM3d.h"

SkinnedMesh::SkinnedMesh(std::unique_ptr<MeshGeometry>& geometry,
                        std::unique_ptr<MeshMaterial>& materials,
                        SkinnedData* skinningData, 
                        std::vector<DirectX::XMFLOAT4X4>& boneTransforms)
{
    geometry = std::move(geometry); 
    materials = std::move(materials);
    skinningData = skinningData;
    boneTransforms = boneTransforms;
};

SkinnedMesh::SkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    UINT& diffuseSrvHeapIndex, UINT& normalSrvHeapIndex, UINT& matCBIndex, 
    std::unordered_map<std::string, std::string>& textureDefinitions, 
    const std::string& filePath)
{
    LoadSkinnedMesh(device, commandList, diffuseSrvHeapIndex, normalSrvHeapIndex, matCBIndex, textureDefinitions, filePath);
}

void SkinnedMesh::UpdateTransforms(const std::string& animClip, float& clipTime)
{
    if (skinningData)
    {
        clipTime++;
        if (clipTime > skinningData->GetClipEndTime(animClip)) { clipTime = 0.0f; }
        skinningData->GetFinalTransforms(animClip, clipTime, boneTransforms);
    }
}

void SkinnedMesh::LoadSkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    UINT& diffuseSrvHeapIndex, UINT& normalSrvHeapIndex, UINT& matCBIndex, 
    std::unordered_map<std::string, std::string>& textureDefinitions,
    const std::string& filePath)
{
    std::vector<M3DLoader::SkinnedVertex> vertices;
    std::vector<std::uint16_t> indices;

    std::vector<M3DLoader::Subset> subsets;
    std::vector<M3DLoader::M3dMaterial> mats;

    SkinnedData skinningInfo;

    M3DLoader m3dLoader;
    m3dLoader.LoadM3d(filePath, vertices, indices, subsets, mats, skinningInfo);

    this->skinningData = &skinningInfo;
    this->boneTransforms.resize(skinningData->BoneCount());

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(M3DLoader::SkinnedVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = filePath;

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(M3DLoader::SkinnedVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    for (UINT i = 0; i < (UINT)subsets.size(); ++i)
    {
        SubmeshGeometry submesh;
        std::string name = "sm_" + std::to_string(i);

        submesh.IndexCount = (UINT)subsets[i].FaceCount * 3;
        submesh.StartIndexLocation = subsets[i].FaceStart * 3;
        submesh.BaseVertexLocation = 0;

        geo->DrawArgs[name] = submesh;
    }

    for (UINT i = 0; i < mats.size(); ++i)
    {
        std::unique_ptr<Material> mat = std::make_unique<Material>();
        std::string name = "m_" + std::to_string(i);

        mat->Name = name;
        mat->MatCBIndex = matCBIndex++;
        mat->DiffuseSrvHeapIndex = diffuseSrvHeapIndex++;
        mat->NormalSrvHeapIndex = normalSrvHeapIndex++;
        mat->DiffuseAlbedo = mats[i].DiffuseAlbedo;
        mat->FresnelR0 = mats[i].FresnelR0;
        mat->Roughness = mats[i].Roughness;

        std::string diffuseName = mats[i].DiffuseMapName;
        std::string normalName = mats[i].NormalMapName;
        std::string diffuseFilename = "Textures/" + diffuseName;
        std::string normalFilename = "Textures/" + normalName;

        diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
        normalName = normalName.substr(0, normalName.find_last_of("."));

        textureDefinitions[diffuseName] = diffuseFilename;
        textureDefinitions[normalName] = normalFilename;

        materials->mappedMaterials[mat->Name] = (std::move(mat));
    }

    geometry = std::move(geo);
}