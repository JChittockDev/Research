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
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices, std::vector<std::vector<Vertex>>& segmentedVertices);
    void ReadSkinningData(unsigned int numMesh, aiMesh** meshList, std::shared_ptr<Skeleton>& mSkeleton, std::vector<Vertex>& vertices, std::vector<SkinningInfo>& skinning);
    void ReadSubsetTable(const aiScene* scene,std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh);
    void ReadSubsetTable(const aiScene* scene, const aiScene* simScene, std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh);
    void ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<UINT>& indices, std::vector<std::vector<UINT>>& segmentedIndices);
    void ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<UINT>& indices);
    void ReadMaterials(const aiScene* scene, std::vector<std::shared_ptr<ModelMaterial>>& mats);
    void ReadSkeleton(const aiScene* scene, std::shared_ptr<Skeleton>& mSkeleton);
    void ReadAnimations(const aiScene* scene, std::unordered_map<std::string, std::shared_ptr<Animation>>& animations);
    void GetSolverConstraints(std::unordered_map<UINT, std::vector<UINT>>& solverConstraints, const std::vector<UINT>& triangles);
    void AddKey(std::unordered_map<UINT, std::vector<UINT>>& map, const UINT& keyA, const UINT& keyB, bool allowPalindromes);
    void GetGraphData(const int count, const std::unordered_map<UINT, std::vector<UINT>>& inputGraph, std::vector<Neighbours>& graphData, const int references);
    void GetSegmentedConstraints(unsigned int numMesh, aiMesh** meshList, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Neighbours>& output);
    void GetVertexTriangleMap(std::unordered_map<UINT, std::vector<UINT>>& vertexTriangleMap, const std::vector<UINT>& triangles);
    void GetSegmentedTriangleMap(unsigned int numMesh, aiMesh** meshList, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Neighbours>& output);
    void GetMeshTransferMap(const std::vector<std::vector<Vertex>>& baseVertices, const std::vector<std::vector<Vertex>>& targetVertices, std::vector<UINT>& map);
};