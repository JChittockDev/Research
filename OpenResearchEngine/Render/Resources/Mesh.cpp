#include "Mesh.h"

void Mesh::ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices)
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

void Mesh::ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<SkinnedVertex>& vertices)
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
}

void Mesh::ReadSkinningData(unsigned int numMesh, aiMesh** meshList, Skeleton* mSkeleton, std::vector<SkinnedVertex>& vertices)
{
	int vertexCounter = 0;
	std::vector<SkinWeight> dynamicWeights(vertices.size());
	for (UINT x = 0; x < numMesh; ++x)
	{
		int numVertices = meshList[x]->mNumVertices;
		for (UINT i = 0; i < meshList[x]->mNumBones; ++i)
		{
			aiVertexWeight* weights = meshList[x]->mBones[i]->mWeights;
			const unsigned int& numWeights = meshList[x]->mBones[i]->mNumWeights;
			std::string boneName(meshList[x]->mBones[i]->mName.C_Str());

			for (UINT w = 0; w < numWeights; ++w)
			{
				int vertexID = vertexCounter + weights[w].mVertexId;
				dynamicWeights[vertexID].BoneWeights.push_back(weights[w].mWeight);
				dynamicWeights[vertexID].BoneIndices.push_back(mSkeleton->bones[boneName].index);
			}
		}
		vertexCounter += numVertices;
	}

	for (UINT x = 0; x < dynamicWeights.size(); ++x)
	{
		SkinWeight& weightData = dynamicWeights[x];
		int remainder = 4 - weightData.BoneWeights.size();

		if (remainder > 0)
		{
			for (UINT y = 0; y < remainder; ++y)
			{
				weightData.BoneWeights.push_back(0.0);
				weightData.BoneIndices.push_back(0);
			}
		}

		vertices[x].BoneWeights = DirectX::XMFLOAT4(weightData.BoneWeights[0], weightData.BoneWeights[1], weightData.BoneWeights[2], weightData.BoneWeights[3]);
		for (UINT y = 0; y < 4; ++y)
		{
			vertices[x].BoneIndices[y] = weightData.BoneIndices[y];
		}
	}

}

void Mesh::ReadSubsetTable(const aiScene* scene, std::vector<Subset>& subsets)
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

void Mesh::ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<USHORT>& indices)
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

void Mesh::ReadMaterials(const aiScene* scene, std::vector<ModelMaterial>& mats)
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

void Mesh::ReadSkeleton(const aiScene* scene, Skeleton* mSkeleton)
{
	unsigned int numMesh = scene->mNumMeshes;
	aiMesh** meshList = scene->mMeshes;

	int boneCount = 0;
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumBones; ++i)
		{
			std::string boneName(meshList[x]->mBones[i]->mName.C_Str());
			bool exists = mSkeleton->bones.find(boneName) != mSkeleton->bones.end();

			if (!exists)
			{
				aiMatrix4x4& offsetMatrix = meshList[x]->mBones[i]->mOffsetMatrix;
				Joint joint(boneName, boneCount, offsetMatrix);
				mSkeleton->bones[boneName] = joint;
				boneCount += 1;
			}
		}
	}
}

void Mesh::ReadAnimations(const aiScene* scene, std::unordered_map<std::string, aiAnimation*> animations)
{
	unsigned int numAnim = scene->mNumAnimations;

	for (UINT x = 0; x < numAnim; ++x)
	{
		animations[scene->mAnimations[x]->mName.C_Str()] = scene->mAnimations[x];
	}
}

Mesh::Mesh(std::string filename, std::string animClip, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice, 
										Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, 
										std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries, 
										std::vector<Subset>& subsets, std::vector<ModelMaterial>& mats)
{
	std::vector<SkinnedVertex> vertices;
	std::vector<std::uint16_t> indices;

	Assimp::Importer* imp = new Assimp::Importer();

	//imp->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	//imp->SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);

	const aiScene* scene = imp->ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
	
	Skeleton* mSkeleton = new Skeleton();
	ReadSkeleton(scene, mSkeleton);
	ReadMaterials(scene, mats);
	ReadSubsetTable(scene, subsets);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices);
	ReadSkinningData(scene->mNumMeshes, scene->mMeshes, mSkeleton, vertices);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices);

	mAnimation = new Animation();
	mAnimation->skeleton = mSkeleton;
	mAnimation->transforms.resize(mSkeleton->bones.size());
	mAnimation->TimePos = 0.0f;
	mAnimation->Speed = 10.0f;
	mAnimation->Loop = true;
	mAnimation->rootNode = scene->mRootNode;
	mAnimation->animation = scene->mAnimations[0];

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	for (UINT i = 0; i < (UINT)subsets.size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);

		submesh.IndexCount = (UINT)subsets[i].IndexCount;
		submesh.StartIndexLocation = subsets[i].IndexStart;
		submesh.BaseVertexLocation = subsets[i].VertexStart;
		submesh.MaterialIndex = subsets[i].MaterialIndex;
		geo->DrawArgs[name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}

void Mesh::UpdateSkinnedCB(float time, std::vector<DirectX::XMFLOAT4X4>& cbTransforms)
{
	mAnimation->UpdateSkinnedAnimation(time);
	cbTransforms = mAnimation->transforms;
}