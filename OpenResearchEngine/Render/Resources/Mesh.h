#pragma once

#include "../../Common/Structures.h"
#include "../../Render/Resources/Skeleton.h"
#include "../../Render/Resources/Animation.h"
#include "../../Render/Resources/FrameResource.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Mesh
{
public:
    Mesh() {};

    Mesh(std::string filename, std::string animClip, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
        std::unordered_map<std::string, std::vector<Subset>>& subsets, std::vector<ModelMaterial>& mats);

    void UpdateSkinnedCB(float time, std::vector<DirectX::XMFLOAT4X4>& cbTransforms);
    Animation* mAnimation;
    const aiScene* scene;

private:
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices);
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<SkinnedVertex>& vertices);
    void ReadSkinningData(unsigned int numMesh, aiMesh** meshList, Skeleton* mSkeleton, std::vector<SkinnedVertex>& vertices);
    void ReadSubsetTable(const aiScene* scene, std::unordered_map<std::string, std::vector<Subset>>& subsets, const std::string& mesh);
    void ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<USHORT>& indices);
    void ReadMaterials(const aiScene* scene, std::vector<ModelMaterial>& mats);
    void ReadSkeleton(const aiScene* scene, Skeleton* mSkeleton);
    void ReadAnimations(const aiScene* scene, std::unordered_map<std::string, aiAnimation*> animations);
    int FindAnimIndex(int numAnim, aiAnimation** animations, const std::string& animationName);
};