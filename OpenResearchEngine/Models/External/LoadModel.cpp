#include "LoadModel.h"
 
using namespace DirectX;

//accumulate the vertices of different aiMesh into single vector

void ModelLoader::ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumVertices; ++i)
		{
			Vertex vertex;
			if (meshList[x]->HasPositions())
			{
				vertex.Pos = *reinterpret_cast<DirectX::XMFLOAT3*>(&meshList[x]->mVertices[i]);
			}
			if (meshList[x]->HasNormals())
			{
				vertex.Normal = *reinterpret_cast<DirectX::XMFLOAT3*>(&meshList[x]->mNormals[i]);
			}
			if (meshList[x]->HasTangentsAndBitangents())
			{
				vertex.TangentU = *reinterpret_cast<DirectX::XMFLOAT4*>(&meshList[x]->mTangents[i]);
			}
			if (meshList[x]->HasTextureCoords(0))
			{
				vertex.TexC = *reinterpret_cast<DirectX::XMFLOAT2*>(&meshList[x]->mTextureCoords[0][i]);
			}
			vertices.push_back(vertex);
		}
	}
}

void ModelLoader::ReadSkinnedVertices(unsigned int numMesh, aiMesh** meshList, std::unordered_map<std::string, int> boneIndex, std::vector<SkinnedVertex>& vertices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumVertices; ++i)
		{
			SkinnedVertex vertex;

			if (meshList[x]->HasPositions())
			{
				vertex.Pos = *reinterpret_cast<DirectX::XMFLOAT3*>(&meshList[x]->mVertices[i]);
			}
			if (meshList[x]->HasNormals())
			{
				vertex.Normal = *reinterpret_cast<DirectX::XMFLOAT3*>(&meshList[x]->mNormals[i]);
			}
			if (meshList[x]->HasTangentsAndBitangents())
			{
				vertex.TangentU = *reinterpret_cast<DirectX::XMFLOAT4*>(&meshList[x]->mTangents[i]);
			}
			if (meshList[x]->HasTextureCoords(0))
			{
				vertex.TexC = *reinterpret_cast<DirectX::XMFLOAT2*>(&meshList[x]->mTextureCoords[0][i]);
			}
			vertices.push_back(vertex);
		}
	}
	ReadSkinningData(numMesh, meshList, boneIndex, vertices);
}

void ModelLoader::ReadSkinningData(unsigned int numMesh, aiMesh** meshList, std::unordered_map<std::string, int> boneIndex, std::vector<SkinnedVertex>& vertices)
{

	int vertexCounter = 0;
	std::vector<SkinWeight> weightTable(vertices.size());
	for (UINT x = 0; x < numMesh; ++x)
	{
		int numVertices = meshList[x]->mNumVertices;
		for (UINT i = 0; i < meshList[x]->mNumBones; ++i)
		{
			aiVertexWeight* weights = meshList[x]->mBones[i]->mWeights;
			const unsigned int& numWeights = meshList[x]->mBones[i]->mNumWeights;

			aiString& boneName = meshList[x]->mBones[i]->mName;
			std::string cBoneName(boneName.C_Str());

			for (UINT w = 0; w < numWeights; ++w)
			{
				int vertexID = vertexCounter + weights[w].mVertexId;
				weightTable[vertexID].BoneWeights.push_back(weights[w].mWeight);
				weightTable[vertexID].BoneIndices.push_back(boneIndex[cBoneName]);
			}
		}
		vertexCounter += numVertices;
	}

	for (UINT x = 0; x < weightTable.size(); ++x)
	{
		SkinWeight& weightData = weightTable[x];
		int remainder = 4 - weightData.BoneWeights.size();

		if (remainder > 0)
		{
			for (UINT y = 0; y < remainder; ++y)
			{
				weightData.BoneWeights.push_back(0.0);
				weightData.BoneIndices.push_back(0);
			}
		}

		vertices[x].BoneWeights = XMFLOAT4(weightData.BoneWeights[0], weightData.BoneWeights[1], weightData.BoneWeights[2], weightData.BoneWeights[3]);
		for (UINT y = 0; y < 4; ++y)
		{
			vertices[x].BoneIndices[y] = weightData.BoneIndices[y];
		}
	}

}

void ModelLoader::ReadSubsetTable(const aiScene* scene, std::vector<Subset>& subsets)
{
	int vertexCounter = 0;
	int indexCounter = 0;
	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		Subset sb;
		int numVertices = scene->mMeshes[i]->mNumVertices;
		int numIndicies = scene->mMeshes[i]->mNumFaces * 3;
		
		sb.Id = i;
		sb.VertexStart = vertexCounter;
		sb.VertexCount = numVertices;
		sb.IndexStart = indexCounter;
		sb.IndexCount = numIndicies;
		sb.MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		vertexCounter += numVertices;
		indexCounter += numIndicies;

		subsets.push_back(sb);
	}
}

void ModelLoader::ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<USHORT>& indices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumFaces; ++i)
		{
			const aiFace& face = meshList[x]->mFaces[i];
			indices.push_back((USHORT)face.mIndices[0]);
			indices.push_back((USHORT)face.mIndices[1]);
			indices.push_back((USHORT)face.mIndices[2]);
		}
	}
}

void ModelLoader::ReadBoneOffsets(unsigned int numMesh, aiMesh** meshList, std::unordered_map<std::string, int>& boneIndex, std::vector<aiMatrix4x4>& boneOffsets)
{
	boneOffsets.resize(boneIndex.size());

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumBones; ++i)
		{
			aiString& boneName = meshList[x]->mBones[i]->mName;
			std::string cBoneName(boneName.C_Str());
			boneOffsets[boneIndex[cBoneName]] = meshList[x]->mBones[i]->mOffsetMatrix.Transpose();

		}
	}
}

void ModelLoader::BoneParentRecursion(aiNode* boneNode, const std::string& boneName, std::string& parentName)
{
	aiNode* boneParentNode = boneNode->mParent;

	if (boneParentNode != nullptr)
	{
		parentName = boneParentNode->mName.C_Str();
		bool found = parentName.find(boneName) != std::string::npos;

		if (found)
		{
			BoneParentRecursion(boneParentNode, boneName, parentName);
		}
	}
	else
	{
		parentName = "RootNode";
	}
}

void ModelLoader::ReadBoneHierarchy(const aiScene* scene, std::unordered_map<std::string, int> boneIndex, std::unordered_map<std::string, Node>& boneTree, std::string& rootBone)
{
	unsigned int numMesh = scene->mNumMeshes;
	aiMesh** meshList = scene->mMeshes;


	for (const auto& bone : boneIndex)
	{
		Node boneNode;
		boneNode.name = bone.first;
		boneTree[bone.first] = boneNode;
	}

	for (const auto& bone : boneIndex)
	{
		aiString boneName(bone.first);
		aiNode* boneNode = scene->mRootNode->FindNode(boneName);

		std::string parentName;
		BoneParentRecursion(boneNode, bone.first, parentName);
		if (parentName != "RootNode")
		{
			boneTree[bone.first].parent = parentName;
			boneTree[parentName].children.push_back(bone.first);
		}
		else
		{
			rootBone = bone.first;
		}
	}
}

void ModelLoader::ReadAnimationClips(const aiScene* scene, std::unordered_map<std::string, AnimationClip>& outAnimations)
{
	unsigned int numAnimationClips = scene->mNumAnimations;
	aiAnimation** animations = scene->mAnimations;

	for (UINT ci = 0; ci < numAnimationClips; ++ci)
	{
		AnimationClip clip;
		std::string clipName = animations[ci]->mName.C_Str();
		ReadBoneKeyframes(animations[ci], clip.BoneAnimations);
		outAnimations[clipName] = clip;
	}
}

void ModelLoader::ReadBoneKeyframes(aiAnimation* inNodeAnimation, std::unordered_map<std::string, BoneAnimation>& outNodeAnimations)
{
	auto numNodes = inNodeAnimation->mNumChannels;
	auto nodeAnimations = inNodeAnimation->mChannels;

	for (UINT i = 0; i < numNodes; ++i)
	{
		BoneAnimation channelAnim;
		auto numTranslationKeys = nodeAnimations[i]->mNumPositionKeys;
		auto numRotationKeys = nodeAnimations[i]->mNumRotationKeys;
		auto numScalingKeys = nodeAnimations[i]->mNumScalingKeys;

		channelAnim.PositionKeyframes.resize(numTranslationKeys);
		channelAnim.RotationKeyframes.resize(numRotationKeys);
		channelAnim.ScaleKeyframes.resize(numScalingKeys);

		for (UINT k = 0; k < numTranslationKeys; ++k)
		{
			channelAnim.PositionKeyframes[k].TimePos = nodeAnimations[i]->mPositionKeys[k].mTime;
			channelAnim.PositionKeyframes[k].Translation = *reinterpret_cast<DirectX::XMFLOAT3*>(&nodeAnimations[i]->mPositionKeys[k].mValue);
		}

		for (UINT k = 0; k < numRotationKeys; ++k)
		{
			channelAnim.RotationKeyframes[k].TimePos = nodeAnimations[i]->mRotationKeys[k].mTime;
			channelAnim.RotationKeyframes[k].Rotation = *reinterpret_cast<DirectX::XMFLOAT4*>(&nodeAnimations[i]->mRotationKeys[k].mValue);
		}

		for (UINT k = 0; k < numScalingKeys; ++k)
		{
			channelAnim.ScaleKeyframes[k].TimePos = nodeAnimations[i]->mScalingKeys[k].mTime;
			channelAnim.ScaleKeyframes[k].Scale = *reinterpret_cast<DirectX::XMFLOAT3*>(&nodeAnimations[i]->mScalingKeys[k].mValue);
		}
		
		outNodeAnimations[nodeAnimations[i]->mNodeName.C_Str()] = channelAnim;
	}
}

void ModelLoader::ReadMaterials(const aiScene* scene, std::vector<ModelMaterial>& mats)
{
	unsigned int numMaterials = scene->mNumMaterials;
	mats.resize(numMaterials);

	aiMaterial** materials = scene->mMaterials;
	for (UINT i = 0; i < numMaterials; ++i)
	{
		mats[i].Name = std::string(materials[i]->GetName().C_Str());
		mats[i].MaterialTypeName = "Skinned";
		mats[i].DiffuseMapName = "default_diffuse.dds";
		mats[i].NormalMapName = "default_nmap.dds";
	}
}

bool ModelLoader::LoadModel(const std::string& filename,
							std::vector<Vertex>& vertices,
							std::vector<USHORT>& indices,
							std::vector<Subset>& subsets,
							std::vector<ModelMaterial>& mats)
{
	try {
		Assimp::Importer imp;
		const aiScene* scene = imp.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
		ReadMaterials(scene, mats);
		ReadSubsetTable(scene, subsets);
		ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices);
		ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices);
		return true;
	}
	catch (int error) {
		return false;
	}
}

void ModelLoader::GetBoneIndexData(const aiScene* scene, std::unordered_map<std::string, int>& boneIndex)
{
	unsigned int numMesh = scene->mNumMeshes;
	aiMesh** meshList = scene->mMeshes;

	int boneCount = 0;
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumBones; ++i)
		{
			aiString& boneName = meshList[x]->mBones[i]->mName;
			std::string cBoneName(boneName.C_Str());

			bool exists = boneIndex.find(cBoneName) != boneIndex.end();

			if (!exists)
			{
				boneIndex[cBoneName] = boneCount;
				boneCount += 1;
			}

		}
	}
}

bool ModelLoader::LoadModel(const std::string& filename,
							std::vector<SkinnedVertex>& vertices,
							std::vector<USHORT>& indices,
							std::vector<Subset>& subsets,
							std::vector<ModelMaterial>& mats,
							SkinnedData& skinInfo)
{
	try {
		Assimp::Importer imp;
		const aiScene* scene = imp.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);

		std::string rootBone;
		std::vector<aiMatrix4x4> boneOffsets;
		std::unordered_map<std::string, Node> boneTree;
		std::unordered_map<std::string, int> boneIndex;
		std::unordered_map<std::string, AnimationClip> animations;
		
		GetBoneIndexData(scene, boneIndex);
		ReadMaterials(scene, mats);
		ReadSubsetTable(scene, subsets);
		ReadSkinnedVertices(scene->mNumMeshes, scene->mMeshes, boneIndex, vertices);
		ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices);
		ReadBoneOffsets(scene->mNumMeshes, scene->mMeshes, boneIndex, boneOffsets);
		ReadBoneHierarchy(scene, boneIndex, boneTree, rootBone);
		ReadAnimationClips(scene, animations);

		skinInfo.Set(boneTree, boneIndex, boneOffsets, animations, rootBone, scene->mRootNode->mTransformation);

		return true;
	}
	catch (int error) {
		return false;
	}
}