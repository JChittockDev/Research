#include "SkinnedMesh.h"
#include "../Models/External/LoadM3d.h"

void SkinnedMesh::LoadSkinnedMesh(Microsoft::WRL::ComPtr<ID3D12Device>& device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    UINT& srvHeapIndex, UINT& matCBIndex, const std::string& filePath)
{
    std::vector<M3DLoader::SkinnedVertex> vertices;
    std::vector<std::uint16_t> indices;

    std::vector<M3DLoader::Subset> subsets;
    std::vector<M3DLoader::M3dMaterial> mats;

    SkinnedData skinningInfo;

    M3DLoader m3dLoader;
    m3dLoader.LoadM3d(filePath, vertices, indices, subsets, mats, skinningInfo);

    this->_skinningData = &skinningInfo;
    this->_boneTransforms.resize(_skinningData->BoneCount());

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(M3DLoader::SkinnedVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_shared<MeshGeometry>();
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
        std::shared_ptr<Material> mat = std::make_shared<Material>();
        mat->Name = mats[i].Name;
        mat->MatCBIndex = matCBIndex++;
        mat->DiffuseSrvHeapIndex = srvHeapIndex++;
        mat->NormalSrvHeapIndex = srvHeapIndex++;
        mat->DiffuseAlbedo = mats[i].DiffuseAlbedo;
        mat->FresnelR0 = mats[i].FresnelR0;
        mat->Roughness = mats[i].Roughness;

        _materials.push_back(std::move(mat));
    }

    _geometry = std::move(geo);
}