#pragma once

#include "../../Common/Structures.h"
#include "../../Render/Resources/Skeleton.h"
#include "../../Render/Resources/Animation.h"
#include "../../Render/Resources/TransformNode.h"
#include "../../Render/Resources/FrameResource.h"

class Mesh
{
public:
    Mesh() {};

    Mesh(std::string filename, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
        std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
        std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
        std::vector<std::shared_ptr<ModelMaterial>>& mats,
        std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons,
        std::unordered_map<std::string, std::shared_ptr<Animation>>& animations,
        std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms);

    Mesh(std::string filename, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
        std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
        std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
        std::vector<std::shared_ptr<ModelMaterial>>& mats);

private:
    void ReadTransformNodes(aiNode* node, std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms);
    void LinkTransformNodes(aiNode* node, std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms);
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices);
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<SkinnedVertex>& vertices);
    void ReadSkinningData(unsigned int numMesh, aiMesh** meshList, std::shared_ptr<Skeleton>& mSkeleton, std::vector<SkinnedVertex>& vertices);
    void ReadSubsetTable(const aiScene* scene, std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh);
    void ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<USHORT>& indices);
    void ReadMaterials(const aiScene* scene, std::vector<std::shared_ptr<ModelMaterial>>& mats);
    void ReadSkeleton(const aiScene* scene, std::shared_ptr<Skeleton>& mSkeleton);
    void ReadAnimations(const aiScene* scene, std::unordered_map<std::string, std::shared_ptr<Animation>>& animations);
};