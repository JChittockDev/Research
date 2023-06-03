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

void Mesh::ReadVertices(unsigned int numMesh, aiMesh** meshList, std::vector<Vertex>& vertices, std::vector<std::vector<Vertex>>& segmentedVertices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::vector<Vertex> vertexSegment;
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
			vertexSegment.push_back(vertex);
		}
		segmentedVertices.push_back(vertexSegment);
	}
}

void Mesh::ReadSkinningData(unsigned int numMesh, aiMesh** meshList, std::shared_ptr<Skeleton>& mSkeleton, std::vector<Vertex>& vertices, std::vector<SkinningInfo>& skinning)
{
	int vertexCounter = 0;
	std::vector<SkinWeight> dynamicWeights(vertices.size());
	std::vector<SkinningInfo> skinningWeights(vertices.size());

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
				dynamicWeights[vertexID].BoneIndices.push_back(mSkeleton->bones[boneName]->index);
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

		skinningWeights[x].BoneWeights = DirectX::XMFLOAT4(weightData.BoneWeights[0], weightData.BoneWeights[1], weightData.BoneWeights[2], weightData.BoneWeights[3]);
		skinningWeights[x].BoneIndices = DirectX::XMFLOAT4(weightData.BoneIndices[0], weightData.BoneIndices[1], weightData.BoneIndices[2], weightData.BoneIndices[3]);
	}

	skinning = skinningWeights;
}

void Mesh::ReadSubsetTable(const aiScene* scene, const aiScene* simScene, std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh)
{
	int vertexCounter = 0;
	int indexCounter = 0;
	int triangleCounter = 0;
	int simMeshVertexCounter = 0;
	int simMeshIndexCounter = 0;
	int simMeshTriangleCounter = 0;
	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		std::shared_ptr<Subset> sb = std::make_shared<Subset>();
		int numVertices = scene->mMeshes[i]->mNumVertices;
		int numTriangles = scene->mMeshes[i]->mNumFaces;
		int numIndicies = numTriangles * 3;
		int simMeshNumVertices = simScene->mMeshes[i]->mNumVertices;
		int simMeshNumTriangles = simScene->mMeshes[i]->mNumFaces;
		int simMeshNumIndicies = simMeshNumTriangles * 3;

		sb->Id = i;
		sb->VertexStart = vertexCounter;
		sb->VertexCount = numVertices;
		sb->IndexStart = indexCounter;
		sb->IndexCount = numIndicies;
		sb->TriangleStart = triangleCounter;
		sb->TriangleCount = numTriangles;
		sb->SimMeshVertexStart = simMeshVertexCounter;
		sb->SimMeshVertexCount = simMeshNumVertices;
		sb->SimMeshIndexStart = simMeshIndexCounter;
		sb->SimMeshIndexCount = simMeshNumIndicies;
		sb->SimMeshTriangleStart = simMeshTriangleCounter;
		sb->SimMeshTriangleCount = simMeshNumTriangles;
		sb->MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		sb->MeshName = mesh;

		vertexCounter += numVertices;
		indexCounter += numIndicies;
		triangleCounter += numTriangles;
		simMeshVertexCounter += simMeshNumVertices;
		simMeshIndexCounter += simMeshNumIndicies;
		simMeshTriangleCounter += simMeshNumTriangles;

		subsets[mesh].push_back(std::move(sb));
	}
}

void Mesh::ReadSubsetTable(const aiScene* scene, std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh)
{

	int vertexCounter = 0;
	int indexCounter = 0;
	int triangleCounter = 0;
	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		std::shared_ptr<Subset> sb = std::make_shared<Subset>();
		int numVertices = scene->mMeshes[i]->mNumVertices;
		int numTriangles = scene->mMeshes[i]->mNumFaces;
		int numIndicies = numTriangles * 3;

		sb->Id = i;
		sb->VertexStart = vertexCounter;
		sb->VertexCount = numVertices;
		sb->IndexStart = indexCounter;
		sb->IndexCount = numIndicies;
		sb->TriangleStart = triangleCounter;
		sb->TriangleCount = numTriangles;
		sb->MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		sb->MeshName = mesh;
		
		vertexCounter += numVertices;
		indexCounter += numIndicies;
		triangleCounter += numTriangles;

		subsets[mesh].push_back(std::move(sb));
	}
}

void Mesh::ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<UINT>& indices, std::vector<std::vector<UINT>>& segmentedIndices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::vector<UINT> sectionIndexList;

		for (UINT i = 0; i < meshList[x]->mNumFaces; ++i)
		{
			const aiFace& face = meshList[x]->mFaces[i];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
			sectionIndexList.push_back(face.mIndices[0]);
			sectionIndexList.push_back(face.mIndices[1]);
			sectionIndexList.push_back(face.mIndices[2]);
		}
		segmentedIndices.push_back(sectionIndexList);
	}
}

void Mesh::GetSegmentedConstraints(unsigned int numMesh, const std::vector<std::vector<Vertex>>& segmentedVertices, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Neighbours>& outputNeighbours, std::vector<RestConstraint>& outputRestConstraint)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::unordered_map<UINT, std::vector<UINT>> solverGraph;
		GetSolverConstraints(solverGraph, segmentedIndices[x]);

		std::vector<Neighbours> neighbours;
		std::vector<RestConstraint> restConstraint;
		GetGraphData(segmentedVertices[x], solverGraph, neighbours, restConstraint, 8);

		for (UINT y = 0; y < neighbours.size(); ++y)
		{
			outputNeighbours.push_back(neighbours[y]);
			outputRestConstraint.push_back(restConstraint[y]);
		}
	}
}

void Mesh::GetGraphData(const std::vector<Vertex>& vertices, const std::unordered_map<UINT, std::vector<UINT>>& inputGraph, std::vector<Neighbours>& neighbourData, std::vector<RestConstraint>& restConstraintData, const int references = 8)
{
	for (int x = 0; x < vertices.size(); x++)
	{
		Neighbours neighbours;
		RestConstraint restConstraint;
		for (int y = 0; y < references; y++)
		{
			neighbours.index[y] = x;
			restConstraint.length[y] = 0.0;
		}

		neighbourData.push_back(neighbours);
		restConstraintData.push_back(restConstraint);
	}

	for (const auto& adjacencyData : inputGraph)
	{
		std::vector<UINT> adjacentIndicies = adjacencyData.second;
		if (adjacentIndicies.size() < references)
		{
			int remainder = references - adjacentIndicies.size();
			for (int z = 0; z < remainder; z++)
			{
				adjacentIndicies.push_back(adjacencyData.first);
			}
		}

		for (int y = 0; y < adjacentIndicies.size(); y++)
		{
			neighbourData[adjacencyData.first].index[y] = adjacentIndicies[y];
			restConstraintData[adjacencyData.first].length[y] = Math::Length(vertices[adjacencyData.first].Pos, vertices[adjacentIndicies[y]].Pos);
		}
	}
}

void Mesh::GetSegmentedTriangleMap(unsigned int numMesh, aiMesh** meshList, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Neighbours>& output)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::unordered_map<UINT, std::vector<UINT>> map;
		GetVertexTriangleMap(map, segmentedIndices[x]);

		std::vector<Neighbours> neighbours;
		GetGraphData(meshList[x]->mNumVertices, map, neighbours, 8);

		for (UINT y = 0; y < neighbours.size(); ++y)
		{
			output.push_back(neighbours[y]);
		}
	}
}

void Mesh::GetGraphData(const int count, const std::unordered_map<UINT, std::vector<UINT>>& inputGraph, std::vector<Neighbours>& graphData, const int references = 8)
{
	for (int x = 0; x < count; x++)
	{
		Neighbours neighbours;
		for (int y = 0; y < references; y++)
		{
			neighbours.index[y] = x;
		}

		graphData.push_back(neighbours);
	}

	for (const auto& adjacencyData : inputGraph)
	{
		std::vector<UINT> adjacentIndicies = adjacencyData.second;
		if (adjacentIndicies.size() < references)
		{
			int remainder = references - adjacentIndicies.size();
			for (int z = 0; z < remainder; z++)
			{
				adjacentIndicies.push_back(adjacencyData.first);
			}
		}

		for (int y = 0; y < adjacentIndicies.size(); y++)
		{
			graphData[adjacencyData.first].index[y] = adjacentIndicies[y];
		}
	}
}

void Mesh::GetSolverConstraints(std::unordered_map<UINT, std::vector<UINT>>& solverConstraints, const std::vector<UINT>& triangles)
{
	for (size_t t = 0; t < triangles.size(); t += 3)
	{
		const UINT index1 = triangles[t];
		const UINT index2 = triangles[t + 1];
		const UINT index3 = triangles[t + 2];

		AddKey(solverConstraints, index1, index2, false);
		AddKey(solverConstraints, index1, index3, false);
		AddKey(solverConstraints, index2, index3, false);
	}

}

void Mesh::GetVertexTriangleMap(std::unordered_map<UINT, std::vector<UINT>>& vertexTriangleMap, const std::vector<UINT>& triangles)
{
	UINT triangleCount = 0;
	for (size_t t = 0; t < triangles.size(); t += 3)
	{
		const UINT index1 = triangles[t];
		const UINT index2 = triangles[t + 1];
		const UINT index3 = triangles[t + 2];

		AddKey(vertexTriangleMap, index1, triangleCount, true);
		AddKey(vertexTriangleMap, index2, triangleCount, true);
		AddKey(vertexTriangleMap, index3, triangleCount, true);
		
		triangleCount += 1;
	}

}

bool CheckPalindromes(std::unordered_map<UINT, std::vector<UINT>>& map, const UINT& keyA, const UINT& keyB)
{
	if (map.find(keyB) == map.end())
	{
		return false;
	}
	else if (std::find(map[keyB].begin(), map[keyB].end(), keyA) == map[keyB].end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Mesh::AddKey(std::unordered_map<UINT, std::vector<UINT>>& map, const UINT& keyA, const UINT& keyB, bool allowPalindromes = true)
{
	bool palindromeExists = CheckPalindromes(map, keyA, keyB);

	if (map.find(keyA) == map.end())
	{
		if (!allowPalindromes)
		{
			if (!palindromeExists)
			{
				map[keyA] = { keyB };
			}
		}
		else
		{
			map[keyA] = { keyB };
		}
	}
	else if (std::find(map[keyA].begin(), map[keyA].end(), keyB) == map[keyA].end())
	{
		if (!allowPalindromes)
		{
			if (!palindromeExists)
			{
				map[keyA].push_back(keyB);
			}
		}
		else
		{
			map[keyA].push_back(keyB);
		}
	}
}

void Mesh::ReadTriangles(unsigned int numMesh, aiMesh** meshList, std::vector<UINT>& indices)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT i = 0; i < meshList[x]->mNumFaces; ++i)
		{
			const aiFace& face = meshList[x]->mFaces[i];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}
}

void Mesh::ReadMaterials(const aiScene* scene, std::vector<std::shared_ptr<ModelMaterial>>& mats)
{
	unsigned int numMaterials = scene->mNumMaterials;
	aiMaterial** materials = scene->mMaterials;
	for (UINT i = 0; i < numMaterials; ++i)
	{
		std::shared_ptr<ModelMaterial> mat = std::make_shared<ModelMaterial>();
		mat->Name = std::string(materials[i]->GetName().C_Str());
		mat->MaterialTypeName = "Skinned";
		mat->DiffuseMapName = "default_diffuse.dds";
		mat->NormalMapName = "default_nmap.dds";
		mats.push_back(std::move(mat));
	}
}

void Mesh::ReadSkeleton(const aiScene* scene, std::shared_ptr<Skeleton>& mSkeleton)
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
				DirectX::XMMATRIX inverseBindMatrix = DirectX::XMMATRIX(&meshList[x]->mBones[i]->mOffsetMatrix.a1);
				std::shared_ptr<Joint> joint = std::make_shared<Joint>(boneName, boneCount, inverseBindMatrix);
				mSkeleton->bones[boneName] = std::move(joint);
				boneCount += 1;
			}
		}
	}
}

void Mesh::ReadAnimations(const aiScene* scene, std::unordered_map<std::string, std::shared_ptr<Animation>>& animations)
{
	unsigned int numAnim = scene->mNumAnimations;

	for (UINT x = 0; x < numAnim; ++x)
	{
		std::shared_ptr<Animation> animation = std::make_shared<Animation>();
		animation->name = scene->mAnimations[x]->mName.data;
		animation->duration = scene->mAnimations[x]->mDuration;

		for (UINT y = 0; y < scene->mAnimations[x]->mNumChannels; ++y)
		{
			std::unique_ptr<AnimationNode> animationNode = std::make_unique<AnimationNode>();
			animationNode->name = scene->mAnimations[x]->mChannels[y]->mNodeName.data;

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumPositionKeys; ++z)
			{
				std::unique_ptr<aiVectorKey> positionKey = std::make_unique<aiVectorKey>(scene->mAnimations[x]->mChannels[y]->mPositionKeys[z]);
				animationNode->positionKeys.push_back(std::move(positionKey));
			}

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumRotationKeys; ++z)
			{
				std::unique_ptr<aiQuatKey> rotationKey = std::make_unique<aiQuatKey>(scene->mAnimations[x]->mChannels[y]->mRotationKeys[z]);
				animationNode->rotationKeys.push_back(std::move(rotationKey));
			}

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumScalingKeys; ++z)
			{
				std::unique_ptr<aiVectorKey> scaleKey = std::make_unique<aiVectorKey>(scene->mAnimations[x]->mChannels[y]->mScalingKeys[z]);
				animationNode->scalingKeys.push_back(std::move(scaleKey));
			}
		
			animation->animationNodes[animationNode->name] = (std::move(animationNode));
		}

		animations[scene->mAnimations[x]->mName.data] = std::move(animation);
	}
}

void Mesh::LinkTransformNodes(aiNode* node, std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms)
{
	std::string nodeName = node->mName.data;
	if (node->mParent != nullptr)
	{
		transforms[nodeName]->parent = transforms[node->mParent->mName.data];
	}

	for (UINT x = 0; x < node->mNumChildren; ++x)
	{
		std::string childNodeName = node->mChildren[x]->mName.data;
		transforms[nodeName]->children.push_back(transforms[childNodeName]);
		LinkTransformNodes(node->mChildren[x], transforms);
	}
}

void Mesh::ReadTransformNodes(aiNode* node, std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms)
{
	std::shared_ptr<TransformNode> transformNode = std::make_shared<TransformNode>();
	transformNode->name = node->mName.data;
	transformNode->transform = DirectX::XMMATRIX(&node->mTransformation.a1);
	transforms[transformNode->name] = std::move(transformNode);

	for (UINT x = 0; x < node->mNumChildren; ++x)
	{
		ReadTransformNodes(node->mChildren[x], transforms);
	}
}

//void Mesh::GetMeshTransferMap(const std::vector<Vertex>& baseVertices, const std::vector<Vertex>& targetVertices, std::vector<UINT>& map)
//{
//	std::unordered_map<Vector3, UINT> targetVertexMap;
//
//	for (UINT i = 0; i < targetVertices.size(); ++i)
//	{
//		const Vertex& targetVertex = targetVertices[i];
//		Vector3 key(targetVertex.Pos.x, targetVertex.Pos.y, targetVertex.Pos.z);
//		targetVertexMap[key] = i;
//	}
//
//	for (UINT i = 0; i < baseVertices.size(); ++i)
//	{
//		const Vertex& baseVertex = baseVertices[i];
//		Vector3 key(baseVertex.Pos.x, baseVertex.Pos.y, baseVertex.Pos.z);
//
//		auto it = targetVertexMap.find(key);
//		if (it != targetVertexMap.end())
//		{
//			map.push_back(it->second);
//		}
//	}
//}


void Mesh::GetMeshTransferMap(const std::vector<std::vector<Vertex>>& baseVertices, const std::vector<std::vector<Vertex>>& targetVertices, std::vector<UINT>& map)
{
	for (UINT meshIndex = 0; meshIndex < (UINT)baseVertices.size(); ++meshIndex)
	{
		for (UINT vertexID = 0; vertexID < (UINT)baseVertices[meshIndex].size(); ++vertexID)
		{
			const Vertex& baseVertex = baseVertices[meshIndex][vertexID];

			for (UINT targetVertexID = 0; targetVertexID < (UINT)targetVertices[meshIndex].size(); ++targetVertexID)
			{
				const Vertex& targetVertex = targetVertices[meshIndex][targetVertexID];

				if (baseVertex.Pos.x == targetVertex.Pos.x && baseVertex.Pos.y == targetVertex.Pos.y && baseVertex.Pos.z == targetVertex.Pos.z)
				{
					map.push_back(targetVertexID);
					break;
				}
			}
		}
	}
}

Mesh::Mesh(std::string filename, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
	std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
	std::vector<std::shared_ptr<ModelMaterial>>& mats)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	ReadMaterials(scene, mats);
	ReadSubsetTable(scene, subsets, filename);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices);

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	for (UINT i = 0; i < (UINT)subsets[filename].size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);
		submesh.VertexCount = (UINT)subsets[filename][i]->VertexCount;
		submesh.IndexCount = (UINT)subsets[filename][i]->IndexCount;
		submesh.StartIndexLocation = subsets[filename][i]->IndexStart;
		submesh.StartVertexLocation = subsets[filename][i]->VertexStart;
		submesh.StartTriangleLocation = subsets[filename][i]->TriangleStart;
		submesh.MaterialIndex = subsets[filename][i]->MaterialIndex;
		geo->DrawArgs[name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}

Mesh::Mesh(std::string filename, Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
	std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
	std::vector<std::shared_ptr<ModelMaterial>>& mats,
	std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons,
	std::unordered_map<std::string, std::shared_ptr<Animation>>& animations,
	std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms)
{
	std::vector<Vertex> vertices;
	std::vector<SkinningInfo> skinning;
	std::vector<UINT> indices;
	std::vector<std::vector<UINT>> segmentedIndices;
	std::vector<std::vector<Vertex>> segmentedVertices;

	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	std::shared_ptr<Skeleton> mSkeleton = std::make_shared<Skeleton>();
	mSkeleton->rootNodeName = scene->mRootNode->mName.data;
	ReadSkeleton(scene, mSkeleton);
	ReadMaterials(scene, mats);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices, segmentedVertices);
	ReadSkinningData(scene->mNumMeshes, scene->mMeshes, mSkeleton, vertices, skinning);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices, segmentedIndices);
	ReadAnimations(scene, animations);
	ReadTransformNodes(scene->mRootNode, transforms);
	LinkTransformNodes(scene->mRootNode, transforms);
	skeletons[filename] = std::move(mSkeleton);

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT sbByteSize = (UINT)skinning.size() * sizeof(SkinningInfo);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(sbByteSize, &geo->SkinningBufferCPU));
	CopyMemory(geo->SkinningBufferCPU->GetBufferPointer(), skinning.data(), sbByteSize);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->SkinnedVertexBufferCPU));
	CopyMemory(geo->SkinnedVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
	geo->SkinningBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), skinning.data(), sbByteSize, geo->SkinningBufferUploader);
	geo->SkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->SkinnedVertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	CD3DX12_RESOURCE_BARRIER vertexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	mCommandList->ResourceBarrier(1, &vertexBufferBarrier);

	CD3DX12_RESOURCE_BARRIER skinningBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinningBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	mCommandList->ResourceBarrier(1, &skinningBufferBarrier);

	if (geo->Simulation)
	{
		Assimp::Importer simImp;
		simImp.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS | aiComponent_TEXCOORDS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS | aiComponent_TEXTURES | aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_MATERIALS);
		const aiScene* simScene = simImp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveComponent);

		// Simulation Resources

		const int triangleCount = indices.size() / 3;
		std::vector<UINT> meshTransferIndices;
		std::vector<UINT> simMeshTransferIndices;
		std::vector<Vertex> simMeshVertices;
		std::vector<UINT> simMeshIndices;
		std::vector<std::vector<UINT>> simMeshSegmentedIndices;
		std::vector<std::vector<Vertex>> simMeshSegmentedVertices;
		std::vector<Neighbours> simMeshVertexNeighbours;
		std::vector<RestConstraint> simMeshRestConstraints;

		ReadVertices(simScene->mNumMeshes, simScene->mMeshes, simMeshVertices, simMeshSegmentedVertices);
		ReadTriangles(simScene->mNumMeshes, simScene->mMeshes, simMeshIndices, simMeshSegmentedIndices);
		ReadSubsetTable(scene, simScene, subsets, filename);
		GetSegmentedConstraints(simScene->mNumMeshes, simMeshSegmentedVertices, simMeshSegmentedIndices, simMeshVertexNeighbours, simMeshRestConstraints);

		GetMeshTransferMap(segmentedVertices, simMeshSegmentedVertices, meshTransferIndices);
		GetMeshTransferMap(simMeshSegmentedVertices, segmentedVertices, simMeshTransferIndices);

		const UINT smabByteSize = (UINT)simMeshVertexNeighbours.size() * sizeof(Neighbours);
		const UINT smrcbByteSize = (UINT)simMeshRestConstraints.size() * sizeof(RestConstraint);

		const UINT smsbByteSize = (UINT)simMeshVertices.size() * sizeof(Spring);
		const UINT smvbByteSize = (UINT)simMeshVertices.size() * sizeof(Vertex);
		const UINT smtbByteSize = (UINT)simMeshVertices.size() * sizeof(UINT);
		const UINT mtbByteSize = (UINT)vertices.size() * sizeof(UINT);

		std::vector<Spring> simMeshSprings;
		simMeshSprings.resize(simMeshVertices.size());
		
		ThrowIfFailed(D3DCreateBlob(smvbByteSize, &geo->SimMeshRestConstraintBufferCPU));
		CopyMemory(geo->SimMeshRestConstraintBufferCPU->GetBufferPointer(), simMeshRestConstraints.data(), smrcbByteSize);

		ThrowIfFailed(D3DCreateBlob(smvbByteSize, &geo->SimMeshSkinnedVertexBufferCPU));
		CopyMemory(geo->SimMeshSkinnedVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), smvbByteSize);

		ThrowIfFailed(D3DCreateBlob(smabByteSize, &geo->SimMeshVertexAdjacencyBufferCPU));
		CopyMemory(geo->SimMeshVertexAdjacencyBufferCPU->GetBufferPointer(), simMeshVertexNeighbours.data(), smabByteSize);

		ThrowIfFailed(D3DCreateBlob(smvbByteSize, &geo->SimMeshTransformedVertexBufferCPU));
		CopyMemory(geo->SimMeshTransformedVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), smvbByteSize);

		ThrowIfFailed(D3DCreateBlob(smsbByteSize, &geo->SimMeshSpringTransformBufferCPU));
		CopyMemory(geo->SimMeshSpringTransformBufferCPU->GetBufferPointer(), simMeshSprings.data(), smsbByteSize);

		ThrowIfFailed(D3DCreateBlob(smtbByteSize, &geo->SimMeshTransferBufferCPU));
		CopyMemory(geo->SimMeshTransferBufferCPU->GetBufferPointer(), simMeshTransferIndices.data(), smtbByteSize);

		ThrowIfFailed(D3DCreateBlob(mtbByteSize, &geo->MeshTransferBufferCPU));
		CopyMemory(geo->MeshTransferBufferCPU->GetBufferPointer(), meshTransferIndices.data(), mtbByteSize);

		geo->SimMeshRestConstraintBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshRestConstraints.data(), smrcbByteSize, geo->SimMeshRestConstraintBufferUploader);
		geo->SimMeshSkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), smvbByteSize, geo->SimMeshSkinnedVertexBufferUploader);
		geo->SimMeshVertexAdjacencyBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertexNeighbours.data(), smabByteSize, geo->SimMeshVertexAdjacencyBufferUploader);
		geo->SimMeshTransformedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), smvbByteSize, geo->SimMeshTransformedVertexBufferUploader);
		geo->SimMeshSpringTransformBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSprings.data(), smsbByteSize, geo->SimMeshSpringTransformBufferUploader);
		geo->SimMeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshTransferIndices.data(), smtbByteSize, geo->SimMeshTransferBufferUploader);
		geo->MeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), meshTransferIndices.data(), mtbByteSize, geo->MeshTransferBufferUploader);

		CD3DX12_RESOURCE_BARRIER SimMeshRestConstraintBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshRestConstraintBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &SimMeshRestConstraintBufferBarrier);

		CD3DX12_RESOURCE_BARRIER SimMeshSkinnedVertexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &SimMeshSkinnedVertexBufferBarrier);

		CD3DX12_RESOURCE_BARRIER SimMeshVertexAdjacencyBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshVertexAdjacencyBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &SimMeshVertexAdjacencyBufferBarrier);

		CD3DX12_RESOURCE_BARRIER SimMeshTransformedVertexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshTransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &SimMeshTransformedVertexBufferBarrier);

		CD3DX12_RESOURCE_BARRIER SimMeshSpringTranformBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSpringTransformBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		mCommandList->ResourceBarrier(1, &SimMeshSpringTranformBufferBarrier);

		CD3DX12_RESOURCE_BARRIER SimMeshTransferBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &SimMeshTransferBufferBarrier);

		CD3DX12_RESOURCE_BARRIER MeshTransferBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->MeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &MeshTransferBufferBarrier);

		// Normal & Tangent Resources

		std::vector<Neighbours> triangleNeighbours;
		GetSegmentedTriangleMap(scene->mNumMeshes, scene->mMeshes, segmentedIndices, triangleNeighbours);

		const UINT nbByteSize = (UINT)(triangleCount) * sizeof(TangentNormals);
		const UINT abByteSize = (UINT)triangleNeighbours.size() * sizeof(Neighbours);

		std::vector<TangentNormals> normals;
		normals.resize(triangleCount);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->TransformedVertexBufferCPU));
		CopyMemory(geo->TransformedVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
		
		ThrowIfFailed(D3DCreateBlob(abByteSize, &geo->TriangleAdjacencyBufferCPU));
		CopyMemory(geo->TriangleAdjacencyBufferCPU->GetBufferPointer(), triangleNeighbours.data(), abByteSize);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->TriangleNormalBufferCPU));
		CopyMemory(geo->TriangleNormalBufferCPU->GetBufferPointer(), normals.data(), nbByteSize);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexNormalBufferCPU));
		CopyMemory(geo->VertexNormalBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		geo->TransformedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->TransformedVertexBufferUploader);
		geo->TriangleAdjacencyBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), triangleNeighbours.data(), abByteSize, geo->TriangleAdjacencyBufferUploader);
		geo->TriangleNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), normals.data(), nbByteSize, geo->TriangleNormalBufferUploader);
		geo->VertexNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexNormalBufferUploader);
		
		CD3DX12_RESOURCE_BARRIER transformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &transformedBufferBarrier);

		CD3DX12_RESOURCE_BARRIER triangleAdjacencyBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->TriangleAdjacencyBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &triangleAdjacencyBufferBarrier);

		CD3DX12_RESOURCE_BARRIER triangleNormalBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &triangleNormalBufferBarrier);

		CD3DX12_RESOURCE_BARRIER vertexNormalBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		mCommandList->ResourceBarrier(1, &vertexNormalBufferBarrier);

		// Default Resources

		CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		mCommandList->ResourceBarrier(1, &skinnedBufferBarrier);

	}
	else
	{
		ReadSubsetTable(scene, subsets, filename);

		CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		mCommandList->ResourceBarrier(1, &skinnedBufferBarrier);
	}

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	for (UINT i = 0; i < (UINT)subsets[filename].size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);
		
		submesh.VertexCount = (UINT)subsets[filename][i]->VertexCount;
		submesh.IndexCount = (UINT)subsets[filename][i]->IndexCount;
		submesh.TriangleCount = (UINT)subsets[filename][i]->TriangleCount;
		submesh.StartIndexLocation = subsets[filename][i]->IndexStart;
		submesh.StartVertexLocation = subsets[filename][i]->VertexStart;
		submesh.StartTriangleLocation = subsets[filename][i]->TriangleStart;
		submesh.MaterialIndex = subsets[filename][i]->MaterialIndex;
		
		if (geo->Simulation)
		{
			submesh.SimMeshVertexCount = (UINT)subsets[filename][i]->SimMeshVertexCount;
			submesh.SimMeshIndexCount = (UINT)subsets[filename][i]->SimMeshIndexCount;
			submesh.SimMeshTriangleCount = (UINT)subsets[filename][i]->SimMeshTriangleCount;
			submesh.SimMeshStartIndexLocation = subsets[filename][i]->SimMeshIndexStart;
			submesh.SimMeshStartVertexLocation = subsets[filename][i]->SimMeshVertexStart;
			submesh.SimMeshStartTriangleLocation = subsets[filename][i]->SimMeshTriangleStart;
		}

		geo->DrawArgs[name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}