#ifndef LOADModel_H
#define LOADModel_H

#include "../../Common/Structures.h"
#include "../../Render/Resources/SkinnedData.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class ModelLoader
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TexC;
        DirectX::XMFLOAT4 TangentU;
        
        Vertex()
        {
            Pos = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
            Normal = DirectX::XMFLOAT3(0.0, 1.0, 0.0);
            TexC = DirectX::XMFLOAT2(0.0, 0.0);
            TangentU = DirectX::XMFLOAT4(1.0, 0.0, 0.0, 0.0);
        }
    };

    struct SkinnedVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TexC;
        DirectX::XMFLOAT4 TangentU;
        DirectX::XMFLOAT4 BoneWeights;
        BYTE BoneIndices[4];

        SkinnedVertex()
        {
            Pos = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
            Normal = DirectX::XMFLOAT3(0.0, 1.0, 0.0);
            TexC = DirectX::XMFLOAT2(0.0, 0.0);
            TangentU = DirectX::XMFLOAT4(1.0, 0.0, 0.0, 0.0);
        }
    };

    struct Subset
    {
        UINT Id = -1;
        UINT VertexStart = 0;
        UINT VertexCount = 0;
        UINT IndexStart = 0;
        UINT IndexCount = 0;
        UINT MaterialIndex = 0;
    };

    struct ModelMaterial
    {
        std::string Name;

        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
        float Roughness = 0.8f;
        bool AlphaClip = false;

        std::string MaterialTypeName;
        std::string DiffuseMapName;
        std::string NormalMapName;
    };

    struct SkinWeight
    {
        std::vector<float> BoneWeights;
        std::vector<int> BoneIndices;
    };

	bool LoadModel(const std::string& filename, 
		std::vector<Vertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<ModelMaterial>& mats);
	bool LoadModel(const std::string& filename, 
		std::vector<SkinnedVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<ModelMaterial>& mats,
		SkinnedData& skinInfo);

private:
    void ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices);
    void ReadSkinnedVertices(unsigned int numMesh, aiMesh** meshList, std::unordered_map<std::string, int> boneIndex, std::vector<SkinnedVertex>& vertices);
    void ReadSubsetTable(const aiScene* scene, std::vector<Subset>& subsets);
    void ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<USHORT>& indices);
    void ReadBoneOffsets(unsigned int numMesh, aiMesh** meshList, std::unordered_map<std::string, int>& hierachyData, std::vector<aiMatrix4x4>& boneOffsets);
    void ReadBoneHierarchy(const aiScene* scene, std::unordered_map<std::string, int> boneIndex, std::unordered_map<std::string, Node>& boneTree, std::string& rootBone);
    void ReadBoneKeyframes(aiAnimation* inNodeAnimation, std::unordered_map<std::string, BoneAnimation>& outNodeAnimations);
    void ReadAnimationClips(const aiScene* scene, std::unordered_map<std::string, AnimationClip>& outAnimations);
    void ReadMaterials(const aiScene* scene, std::vector<ModelMaterial>& mats);
    void GetBoneIndexData(const aiScene* scene, std::unordered_map<std::string, int>& boneIndex);
    void GenerateWeightTable(std::vector<SkinWeight>& weightTable, const std::vector<SkinnedVertex>& vertices);
    void BoneParentRecursion(aiNode* boneNode, const std::string& boneName, std::string& parentName);
};
#endif // LOADModel_H