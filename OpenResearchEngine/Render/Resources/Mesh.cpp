#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION    
#include "../../Utilities/stb_image.h"

void Mesh::LoadTexture(const char* filepath, int& width, int& height, std::vector<unsigned char>& textureData)
{
	int nrChannels;
	unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 3);
	std::vector<unsigned char> txtData(data, data + (width * height * nrChannels));
	stbi_image_free(data);
	textureData = txtData;
}

void Mesh::ReadVertexColor(const std::vector<unsigned char>& textureData, int texWidth, int texHeight, const std::vector<Vertex>& vertices, std::vector<Vector4>& vertexMasks) {
	
	for (auto& vertex : vertices) {

		float u = (vertex.TexC.x < 0.0f) ? 0.0f : (vertex.TexC.x > 1.0f) ? 1.0f : vertex.TexC.x;
		float v = (vertex.TexC.y < 0.0f) ? 0.0f : (vertex.TexC.y > 1.0f) ? 1.0f : vertex.TexC.y;
		int texX = static_cast<int>(u * (texWidth - 1));
		int texY = static_cast<int>((1.0f - v) * (texHeight - 1));
		int texIndex = (texY * texWidth + texX) * 3;

		Vector4 vertexMask(textureData[texIndex] * 1.0f / 255.0f, textureData[texIndex + 1] * 1.0f / 255.0f, textureData[texIndex + 2] * 1.0f / 255.0f, 1.0f);

		vertexMasks.push_back(vertexMask);
	}
}

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

void Mesh::ReadBlendshapeData(unsigned int numMesh, aiMesh** meshList, std::vector<BlendshapeVertex>& blendshapes)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		for (UINT y = 0; y < meshList[x]->mNumAnimMeshes; ++y)
		{
			for (UINT z = 0; z < meshList[x]->mAnimMeshes[y]->mNumVertices; ++z)
			{
				BlendshapeVertex blendVertex;
				blendVertex.index = z;
				if (meshList[x]->mAnimMeshes[y]->mVertices != NULL)
				{
					aiVector3D deltaPosition = meshList[x]->mAnimMeshes[y]->mVertices[z] - meshList[x]->mVertices[z];
					blendVertex.position = { deltaPosition.x, deltaPosition.y, deltaPosition.z };
				}
				if (meshList[x]->mAnimMeshes[y]->mNormals != NULL)
				{
					aiVector3D deltaNormal = meshList[x]->mAnimMeshes[y]->mNormals[z] - meshList[x]->mNormals[z];
					blendVertex.normal = { deltaNormal.x, deltaNormal.y, deltaNormal.z };
				}
				if (meshList[x]->mAnimMeshes[y]->mTangents != NULL)
				{
					aiVector3D deltaTangent = meshList[x]->mAnimMeshes[y]->mTangents[z] - meshList[x]->mTangents[z];
					blendVertex.tangent = { deltaTangent.x, deltaTangent.y, deltaTangent.z };
				}
				blendshapes.push_back(blendVertex);
			}
		}
	}
}

void Mesh::ReadSubsetTable(const aiScene* scene, const aiScene* simScene, std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets, const std::string& mesh)
{
	int vertexCounter = 0;
	int indexCounter = 0;
	int triangleCounter = 0;
	int blendshapeCounter = 0;
	int blendshapeVertexCounter = 0;
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

		int blendshapeSubsetVertexCounter = 0;
		for (UINT y = 0; y < scene->mMeshes[i]->mNumAnimMeshes; ++y)
		{
			BlendshapeSubset blendSubset;
			int numBlendshapeVertices = scene->mMeshes[i]->mAnimMeshes[y]->mNumVertices;
			blendSubset.VertexCount = numBlendshapeVertices;
			blendSubset.VertexStart = blendshapeSubsetVertexCounter;
			blendshapeSubsetVertexCounter += numBlendshapeVertices;
			sb->BlendshapeSubsets.push_back(blendSubset);
		}

		sb->VertexStart = vertexCounter;
		sb->VertexCount = numVertices;
		sb->IndexStart = indexCounter;
		sb->IndexCount = numIndicies;
		sb->BlendshapeCount = scene->mMeshes[i]->mNumAnimMeshes;
		sb->BlendshapeStart = blendshapeCounter;
		sb->BlendshapeVertexCount = blendshapeSubsetVertexCounter;
		sb->BlendshapeVertexStart = blendshapeVertexCounter;
		sb->TriangleStart = triangleCounter;
		sb->TriangleCount = numTriangles;
		sb->SimMeshVertexStart = simMeshVertexCounter;
		sb->SimMeshVertexCount = simMeshNumVertices;
		sb->SimMeshIndexStart = simMeshIndexCounter;
		sb->SimMeshIndexCount = simMeshNumIndicies;
		sb->SimMeshTriangleStart = simMeshTriangleCounter;
		sb->SimMeshTriangleCount = simMeshNumTriangles;
		sb->Name = scene->mMeshes[i]->mName.C_Str();

		vertexCounter += numVertices;
		indexCounter += numIndicies;
		blendshapeVertexCounter += blendshapeSubsetVertexCounter;
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

		sb->VertexStart = vertexCounter;
		sb->VertexCount = numVertices;
		sb->IndexStart = indexCounter;
		sb->IndexCount = numIndicies;
		sb->TriangleStart = triangleCounter;
		sb->TriangleCount = numTriangles;
		sb->Name = scene->mMeshes[i]->mName.C_Str();
		
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

std::string AutoMatchAnimMesh(std::string& key, unsigned int numMesh, aiMesh** meshList)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::string mesh_name = meshList[x]->mName.C_Str();

		if (key.find(mesh_name) != std::string::npos)
		{
			return mesh_name;
		}
	}
	return key;
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
			std::unique_ptr<TransformAnimNode> transformAnimNode = std::make_unique<TransformAnimNode>();
			transformAnimNode->name = scene->mAnimations[x]->mChannels[y]->mNodeName.data;

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumPositionKeys; ++z)
			{
				std::unique_ptr<aiVectorKey> positionKey = std::make_unique<aiVectorKey>(scene->mAnimations[x]->mChannels[y]->mPositionKeys[z]);
				transformAnimNode->positionKeys.push_back(std::move(positionKey));
			}

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumRotationKeys; ++z)
			{
				std::unique_ptr<aiQuatKey> rotationKey = std::make_unique<aiQuatKey>(scene->mAnimations[x]->mChannels[y]->mRotationKeys[z]);
				transformAnimNode->rotationKeys.push_back(std::move(rotationKey));
			}

			for (UINT z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumScalingKeys; ++z)
			{
				std::unique_ptr<aiVectorKey> scaleKey = std::make_unique<aiVectorKey>(scene->mAnimations[x]->mChannels[y]->mScalingKeys[z]);
				transformAnimNode->scalingKeys.push_back(std::move(scaleKey));
			}
		
			animation->TransformAnimNodes[transformAnimNode->name] = std::move(transformAnimNode);
		}

		UINT counter = 0;
		for (UINT y = 0; y < scene->mAnimations[x]->mNumMorphMeshChannels; ++y)
		{
			std::unique_ptr<BlendAnimNode> blendAnimNode = std::make_unique<BlendAnimNode>();
			blendAnimNode->name = scene->mAnimations[x]->mMorphMeshChannels[y]->mName.data;
			blendAnimNode->blendsStart = counter;

			for (UINT z = 0; z < scene->mAnimations[x]->mMorphMeshChannels[y]->mNumKeys; ++z)
			{
				counter += scene->mAnimations[x]->mMorphMeshChannels[y]->mKeys[z].mNumValuesAndWeights;
				MorphKey weightKey(scene->mAnimations[x]->mMorphMeshChannels[y]->mKeys[z]);
				blendAnimNode->weightKeys.push_back(weightKey);
			}

			animation->BlendAnimNodes[AutoMatchAnimMesh(blendAnimNode->name, scene->mNumMeshes, scene->mMeshes)] = std::move(blendAnimNode);
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

void GetVertexNeighbours(unsigned int numMesh, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<std::vector<VertexNeighbours>>& segmentedVertexNeighbours, const int references = 8)
{
	std::vector<std::unordered_map<UINT, std::set<UINT>>> segmentedVertexNeighbourIDs;
	segmentedVertexNeighbourIDs.resize(numMesh);

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (size_t i = 0; i < segmentedIndices[x].size(); i += 3)
		{
			UINT v0 = segmentedIndices[x][i];
			UINT v1 = segmentedIndices[x][i + 1];
			UINT v2 = segmentedIndices[x][i + 2];

			segmentedVertexNeighbourIDs[x][v0].insert(v1);
			segmentedVertexNeighbourIDs[x][v0].insert(v2);
			segmentedVertexNeighbourIDs[x][v1].insert(v0);
			segmentedVertexNeighbourIDs[x][v1].insert(v2);
			segmentedVertexNeighbourIDs[x][v2].insert(v1);
			segmentedVertexNeighbourIDs[x][v2].insert(v0);
		}
	}

	for (UINT x = 0; x < numMesh; ++x)
	{
		std::vector<VertexNeighbours> vertexNeighbours;

		// for each vertex
		for (size_t i = 0; i < segmentedVertexNeighbourIDs[x].size(); ++i)
		{
			VertexNeighbours neighbourIDs;

			// add each neighbour triangle to neighbours
			int counter = 0;
			for (const UINT& id : segmentedVertexNeighbourIDs[x][i])
			{
				if (counter < references)
				{
					neighbourIDs.index[counter] = id;
					counter += 1;
				}
			}

			if (counter < references)
			{
				int remainder = references - counter;

				for (size_t z = 0; z < remainder; ++z)
				{
					// offset by one as to no override the last neighbour
					neighbourIDs.index[counter + z] = i;
				}
			}
			vertexNeighbours.push_back(neighbourIDs);
		}
		segmentedVertexNeighbours.push_back(vertexNeighbours);
	}
}

void GetVertexNeighbourLengths(std::vector<std::vector<VertexNeighbours>>& segmentedVertexNeighbours, std::vector<std::vector<Vertex>>& segmentedVertices, std::vector<VertexNeighbours>& vertexNeighbours, const int references = 8)
{
	for (UINT x = 0; x < segmentedVertexNeighbours.size(); ++x)
	{
		for (UINT y = 0; y < segmentedVertexNeighbours[x].size(); ++y)
		{
			VertexNeighbours neighbours = segmentedVertexNeighbours[x][y];

			for (UINT z = 0; z < references; ++z)
			{
				UINT secondaryVertexID = neighbours.index[z];
				neighbours.length[z] = Math::Length(segmentedVertices[x][y].Pos, segmentedVertices[x][secondaryVertexID].Pos);
			}

			vertexNeighbours.push_back(neighbours);
		}
	}
}

void GetVertexTriangleNeighbours(unsigned int numMesh, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Neighbours>& vertexNeighbourTriangles, const int references = 8)
{
	std::vector<std::unordered_map<UINT, std::set<UINT>>> segmentedVertexNeighbourTriangles;
	segmentedVertexNeighbourTriangles.resize(numMesh);

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (size_t i = 0; i < segmentedIndices[x].size(); i += 3)
		{
			UINT v0 = segmentedIndices[x][i];
			UINT v1 = segmentedIndices[x][i + 1];
			UINT v2 = segmentedIndices[x][i + 2];

			segmentedVertexNeighbourTriangles[x][v0].insert(i / 3);
			segmentedVertexNeighbourTriangles[x][v1].insert(i / 3);
			segmentedVertexNeighbourTriangles[x][v2].insert(i / 3);
		}
	}

	for (UINT x = 0; x < numMesh; ++x)
	{
		// for each vertex
		for (size_t i = 0; i < segmentedVertexNeighbourTriangles[x].size(); ++i)
		{
			Neighbours neighbourIDs;

			// add each neighbour triangle to neighbours
			int counter = 0;
			for (const UINT& id : segmentedVertexNeighbourTriangles[x][i])
			{
				if (counter < references)
				{
					neighbourIDs.index[counter] = id;
					counter += 1;
				}
			}

			if (counter < references)
			{
				int remainder = references - counter;

				for (size_t z = 0; z < remainder; ++z)
				{
					// offset by one as to no override the last neighbour
					neighbourIDs.index[counter + z] = i;
				}
			}
			vertexNeighbourTriangles.push_back(neighbourIDs);
		}
	}
}

void GetStretchEdges(unsigned int numMesh, std::vector<std::shared_ptr<Subset>>& subsets, std::vector<std::vector<Vertex>>& segmentedVertices, std::vector<std::vector<UINT>>& segmentedIndices, 
				std::vector<std::unordered_map<Edge, std::set<UINT>, EdgeHash>>& segmentedEdgeTriangles, std::vector<Edge>& edges, std::vector<float>& restLength)
{
	// Step 1: Create a mapping of each edge to the triangles that share it.

	std::vector<std::set<Edge>> segmentedEdges;
	segmentedEdges.resize(numMesh);
	segmentedEdgeTriangles.resize(numMesh);

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (size_t i = 0; i < segmentedIndices[x].size(); i += 3)
		{
			UINT triangleID = i / 3;

			UINT v0 = segmentedIndices[x][i];
			UINT v1 = segmentedIndices[x][i + 1];
			UINT v2 = segmentedIndices[x][i + 2];

			// Sort vertices to form unique edge key (v0, v1)
			Edge edgeKey1(Math::Min(v0, v1), Math::Max(v0, v1));
			segmentedEdges[x].insert(edgeKey1);
			segmentedEdgeTriangles[x][edgeKey1].insert(triangleID);

			Edge edgeKey2(Math::Min(v1, v2), Math::Max(v1, v2));
			segmentedEdges[x].insert(edgeKey2);
			segmentedEdgeTriangles[x][edgeKey2].insert(triangleID);

			Edge edgeKey3(Math::Min(v2, v0), Math::Max(v2, v0));
			segmentedEdges[x].insert(edgeKey3);
			segmentedEdgeTriangles[x][edgeKey3].insert(triangleID);
		}
	}

	int edgeCounter = 0;
	for (UINT x = 0; x < numMesh; ++x)
	{
		subsets[x]->SimMeshConstraintCount = segmentedEdges[x].size();
		subsets[x]->SimMeshConstraintStart = edgeCounter;

		for (const Edge& edge : segmentedEdges[x])
		{
			Vertex& vertex1 = segmentedVertices[x][edge.vertexA];
			Vertex& vertex2 = segmentedVertices[x][edge.vertexB];
			float length = Math::Length(vertex1.Pos, vertex2.Pos);

			restLength.push_back(length);
			edges.push_back(edge);
		}

		edgeCounter += segmentedEdges[x].size();
	}
}

void GetConstraints(unsigned int numMesh, std::vector<std::shared_ptr<Subset>>& subsets, std::vector<std::vector<Vertex>>& segmentedVertices, 
										std::vector<std::vector<UINT>>& segmentedIndices, std::vector<Edge>& edges, std::vector<float>& restLength)
{
	std::vector<std::set<Edge>> segmentedEdges;
	std::vector<std::unordered_map<Edge, std::set<UINT>, EdgeHash>> segmentedEdgeTriangles;
	segmentedEdges.resize(numMesh);
	segmentedEdgeTriangles.resize(numMesh);

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (size_t i = 0; i < segmentedIndices[x].size(); i += 3)
		{
			UINT triangleID = i / 3;

			UINT v0 = segmentedIndices[x][i];
			UINT v1 = segmentedIndices[x][i + 1];
			UINT v2 = segmentedIndices[x][i + 2];

			// Sort vertices to form unique edge key (v0, v1)
			Edge edgeKey1(Math::Min(v0, v1), Math::Max(v0, v1));
			segmentedEdges[x].insert(edgeKey1);
			segmentedEdgeTriangles[x][edgeKey1].insert(triangleID);

			Edge edgeKey2(Math::Min(v1, v2), Math::Max(v1, v2));
			segmentedEdges[x].insert(edgeKey2);
			segmentedEdgeTriangles[x][edgeKey2].insert(triangleID);

			Edge edgeKey3(Math::Min(v2, v0), Math::Max(v2, v0));
			segmentedEdges[x].insert(edgeKey3);
			segmentedEdgeTriangles[x][edgeKey3].insert(triangleID);
		}
	}

	for (UINT x = 0; x < numMesh; ++x)
	{
		for (size_t i = 0; i < segmentedIndices[x].size(); i += 3)
		{
			UINT v0 = segmentedIndices[x][i];
			UINT v1 = segmentedIndices[x][i + 1];
			UINT v2 = segmentedIndices[x][i + 2];

			UINT triangleIndex = i / 3;
			std::vector<UINT> neighbours;

			// Check neighboring triangles for edge (v0, v1)
			Edge edgeKey1;
			edgeKey1.vertexA = Math::Min(v0, v1);
			edgeKey1.vertexB = Math::Max(v0, v1);

			std::set<UINT> triangles1 = segmentedEdgeTriangles[x][edgeKey1];

			for (UINT triangle : triangles1)
			{
				if (triangle != triangleIndex)
				{
					neighbours.push_back(triangle);
				}
			}

			// Check neighboring triangles for edge (v1, v2)
			Edge edgeKey2;
			edgeKey2.vertexA = Math::Min(v1, v2);
			edgeKey2.vertexB = Math::Max(v1, v2);

			std::set<UINT> triangles2 = segmentedEdgeTriangles[x][edgeKey2];

			for (UINT triangle : triangles2)
			{
				if (triangle != triangleIndex)
				{
					neighbours.push_back(triangle);
				}
			}

			// Check neighboring triangles for edge (v2, v0)
			Edge edgeKey3;
			edgeKey3.vertexA = Math::Min(v2, v0);
			edgeKey3.vertexB = Math::Max(v2, v0);

			std::set<UINT> triangles3 = segmentedEdgeTriangles[x][edgeKey3];

			for (UINT triangle : triangles3)
			{
				if (triangle != triangleIndex)
				{
					neighbours.push_back(triangle);
				}
			}

			std::vector<UINT> source_vertices = { v0, v1, v2 };

			for (UINT y = 0; y < (UINT)neighbours.size(); ++y)
			{
				UINT nv0 = segmentedIndices[x][neighbours[y] * 3];
				UINT nv1 = segmentedIndices[x][neighbours[y] * 3 + 1];
				UINT nv2 = segmentedIndices[x][neighbours[y] * 3 + 2];

				std::vector<UINT> target_vertices = { nv0, nv1, nv2 };

				UINT p1 = nv0;
				UINT p2 = nv1;

				for (UINT z = 0; z < (UINT)target_vertices.size(); ++z)
				{

					if (std::find(source_vertices.begin(), source_vertices.end(), target_vertices[z]) == source_vertices.end())
					{
						p1 = target_vertices[z];
					}
				}

				for (UINT z = 0; z < (UINT)source_vertices.size(); ++z)
				{

					if (std::find(target_vertices.begin(), target_vertices.end(), source_vertices[z]) == target_vertices.end())
					{
						p2 = source_vertices[z];
					}
				}

				Edge edgeKey(Math::Min(p1, p2), Math::Max(p1, p2));
				segmentedEdges[x].insert(edgeKey);
			}
		}
	}

	int edgeCounter = 0;
	for (UINT x = 0; x < segmentedEdges.size(); ++x)
	{
		subsets[x]->SimMeshConstraintCount = segmentedEdges[x].size();
		subsets[x]->SimMeshConstraintStart = edgeCounter;

		for (const Edge& edge : segmentedEdges[x])
		{
			Vertex& vertex1 = segmentedVertices[x][edge.vertexA];
			Vertex& vertex2 = segmentedVertices[x][edge.vertexB];
			float length = Math::Length(vertex1.Pos, vertex2.Pos);

			restLength.push_back(length);
			edges.push_back(edge);
		}

		edgeCounter += segmentedEdges[x].size();
	}
}

Mesh::Mesh(std::string filename, std::string vertexColorFilename, 
	Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
	std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
	bool flipWO)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	Assimp::Importer imp;

	const aiScene* scene;

	if (flipWO)
	{
		scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder);
	}
	else
	{
		scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder);
	}
	
	ReadSubsetTable(scene, subsets, filename);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices);

	const UINT vertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT indexBufferByteSize = (UINT)indices.size() * sizeof(UINT);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;

	ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(indexBufferByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), indexBufferByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->VertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), indexBufferByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vertexBufferByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = indexBufferByteSize;

	for (UINT i = 0; i < (UINT)subsets[filename].size(); ++i)
	{
		SubmeshGeometry submesh;
		submesh.Name = subsets[filename][i]->Name;
		submesh.VertexCount = (UINT)subsets[filename][i]->VertexCount;
		submesh.IndexCount = (UINT)subsets[filename][i]->IndexCount;
		submesh.IndexStart = subsets[filename][i]->IndexStart;
		submesh.VertexStart = subsets[filename][i]->VertexStart;
		submesh.TriangleStart = subsets[filename][i]->TriangleStart;
		geo->DrawArgs[submesh.Name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}

Mesh::Mesh(std::string filename, std::string vertexColorFilename, 
	Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList,
	std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& subsets,
	std::unordered_map<std::string, std::shared_ptr<Skeleton>>& skeletons,
	std::unordered_map<std::string, std::shared_ptr<Animation>>& animations,
	std::unordered_map<std::string, std::shared_ptr<TransformNode>>& transforms)
{
	std::vector<Vertex> vertices;
	std::vector<SkinningInfo> skinning;
	std::vector<UINT> indices;
	std::vector<BlendshapeVertex> blendshapes;
	std::vector<std::vector<UINT>> segmentedIndices;
	std::vector<std::vector<Vertex>> segmentedVertices;

	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	std::shared_ptr<Skeleton> mSkeleton = std::make_shared<Skeleton>();
	mSkeleton->rootNodeName = scene->mRootNode->mName.data;
	ReadSkeleton(scene, mSkeleton);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices, segmentedVertices);

	int vertexColorWidth = 0;
	int vertexColorHeight = 0;
	std::vector<unsigned char> vertexColor;
	LoadTexture(vertexColorFilename.c_str(), vertexColorWidth, vertexColorHeight, vertexColor);

	ReadBlendshapeData(scene->mNumMeshes, scene->mMeshes, blendshapes);
	ReadSkinningData(scene->mNumMeshes, scene->mMeshes, mSkeleton, vertices, skinning);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices, segmentedIndices);
	ReadAnimations(scene, animations);
	ReadTransformNodes(scene->mRootNode, transforms);
	LinkTransformNodes(scene->mRootNode, transforms);
	skeletons[filename] = std::move(mSkeleton);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;

	const UINT vertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT skinningBufferByteSize = (UINT)skinning.size() * sizeof(SkinningInfo);
	const UINT indexBufferByteSize = (UINT)indices.size() * sizeof(UINT);

	ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(skinningBufferByteSize, &geo->SkinningBufferCPU));
	CopyMemory(geo->SkinningBufferCPU->GetBufferPointer(), skinning.data(), skinningBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(indexBufferByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), indexBufferByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->VertexBufferUploader);
	geo->SkinningBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), skinning.data(), skinningBufferByteSize, geo->SkinningBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), indexBufferByteSize, geo->IndexBufferUploader);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinningBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	if (blendshapes.size() > 0)
	{
		const UINT blendshapeBufferByteSize = (UINT)blendshapes.size() * sizeof(BlendshapeVertex);
		ThrowIfFailed(D3DCreateBlob(blendshapeBufferByteSize, &geo->BlendshapeBufferCPU));
		CopyMemory(geo->BlendshapeBufferCPU->GetBufferPointer(), blendshapes.data(), blendshapeBufferByteSize);
		geo->BlendshapeBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), blendshapes.data(), blendshapeBufferByteSize, geo->BlendshapeBufferUploader);
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->BlendshapeBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}

	Assimp::Importer simImp;
	simImp.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS | aiComponent_TEXTURES | aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_MATERIALS);
	const aiScene* simScene = simImp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveComponent);

	// Simulation Resources

	std::vector<Vertex> simMeshVertices;
	std::vector<UINT> simMeshIndices;
	std::vector<std::vector<Vertex>> simMeshSegmentedVertices;
	std::vector<std::vector<UINT>> simMeshSegmentedIndices;
	std::vector<UINT> meshTransferIndices;
	std::vector<UINT> simMeshTransferIndices;
	std::vector<float> simMeshConstraints;
	std::vector<Edge> simMeshConstraintIDs;
	std::vector<std::vector<VertexNeighbours>> segmentedVertexNeighbours;
	std::vector<VertexNeighbours> simMeshVertexNeighbours;

	ReadVertices(simScene->mNumMeshes, simScene->mMeshes, simMeshVertices, simMeshSegmentedVertices);
	ReadTriangles(simScene->mNumMeshes, simScene->mMeshes, simMeshIndices, simMeshSegmentedIndices);
	ReadSubsetTable(scene, simScene, subsets, filename);
	GetMeshTransferMap(segmentedVertices, simMeshSegmentedVertices, meshTransferIndices);
	GetMeshTransferMap(simMeshSegmentedVertices, segmentedVertices, simMeshTransferIndices);
	GetConstraints(simScene->mNumMeshes, subsets[filename], simMeshSegmentedVertices, simMeshSegmentedIndices, simMeshConstraintIDs, simMeshConstraints);

	GetVertexNeighbours(simScene->mNumMeshes, simMeshSegmentedIndices, segmentedVertexNeighbours);
	GetVertexNeighbourLengths(segmentedVertexNeighbours, simMeshSegmentedVertices, simMeshVertexNeighbours);

	std::vector<Vector4> vertexColorData;
	ReadVertexColor(vertexColor, vertexColorWidth, vertexColorHeight, simMeshVertices, vertexColorData);

	const UINT simMeshVertexBufferByteSize = (UINT)simMeshVertices.size() * sizeof(Vertex);
	const UINT simMeshVertexNeighbourBufferByteSize = (UINT)simMeshVertices.size() * sizeof(VertexNeighbours);
	const UINT simMeshNullSolverAccumulationBufferByteSize = (UINT)simMeshVertices.size() * sizeof(UINT3);
	const UINT simMeshNullSolverCountBufferByteSize = (UINT)simMeshVertices.size() * sizeof(UINT);
	const UINT simMeshTransferBufferByteSize = (UINT)simMeshVertices.size() * sizeof(UINT);
	const UINT simMeshConstraintBufferByteSize = (UINT)simMeshConstraints.size() * sizeof(float);
	const UINT simMeshConstraintIDsBufferByteSize = (UINT)simMeshConstraintIDs.size() * sizeof(Edge);
	const UINT meshTransferBufferByteSize = (UINT)vertices.size() * sizeof(UINT);

	const UINT simMeshVertexColorBufferByteSize = (UINT)simMeshVertices.size() * sizeof(Vector4);

	std::vector<UINT3> simMeshSolverAccumulation(simMeshVertices.size());
	std::vector<UINT> simMeshSolverCount(simMeshVertices.size());

	geo->DeformationData.vertices = vertices;
	geo->DeformationData.normals.resize(indices.size() / 3);
	geo->DeformationData.simMeshVertices = simMeshVertices;
	geo->DeformationData.simMeshForce.resize(simMeshVertices.size());
	geo->DeformationData.simMeshTension.resize(simMeshVertices.size());
	geo->DeformationData.simMeshSolverAccumulation.resize(simMeshVertices.size());
	geo->DeformationData.simMeshSolverCount.resize(simMeshVertices.size());

	ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshVertexBufferCPU));
	CopyMemory(geo->SimMeshVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshVertexNeighbourBufferByteSize, &geo->SimMeshVertexNeighbourBufferCPU));
	CopyMemory(geo->SimMeshVertexNeighbourBufferCPU->GetBufferPointer(), simMeshVertexNeighbours.data(), simMeshVertexNeighbourBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshConstraintBufferByteSize, &geo->SimMeshConstraintsBufferCPU));
	CopyMemory(geo->SimMeshConstraintsBufferCPU->GetBufferPointer(), simMeshConstraints.data(), simMeshConstraintBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshConstraintIDsBufferByteSize, &geo->SimMeshConstraintIDsBufferCPU));
	CopyMemory(geo->SimMeshConstraintIDsBufferCPU->GetBufferPointer(), simMeshConstraintIDs.data(), simMeshConstraintIDsBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshNullSolverAccumulationBufferByteSize, &geo->SimMeshNullSolverAccumulationBufferCPU));
	CopyMemory(geo->SimMeshNullSolverAccumulationBufferCPU->GetBufferPointer(), simMeshSolverAccumulation.data(), simMeshNullSolverAccumulationBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshNullSolverCountBufferByteSize, &geo->SimMeshNullSolverCountBufferCPU));
	CopyMemory(geo->SimMeshNullSolverCountBufferCPU->GetBufferPointer(), simMeshSolverCount.data(), simMeshNullSolverCountBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshTransferBufferByteSize, &geo->SimMeshTransferBufferCPU));
	CopyMemory(geo->SimMeshTransferBufferCPU->GetBufferPointer(), simMeshTransferIndices.data(), simMeshTransferBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(meshTransferBufferByteSize, &geo->MeshTransferBufferCPU));
	CopyMemory(geo->MeshTransferBufferCPU->GetBufferPointer(), meshTransferIndices.data(), meshTransferBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(simMeshVertexColorBufferByteSize, &geo->SimMeshVertexColorBufferCPU));
	CopyMemory(geo->SimMeshVertexColorBufferCPU->GetBufferPointer(), vertexColorData.data(), simMeshVertexColorBufferByteSize);

	geo->SimMeshVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshVertexBufferUploader);
	geo->SimMeshVertexNeighbourBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertexNeighbours.data(), simMeshVertexNeighbourBufferByteSize, geo->SimMeshVertexNeighbourBufferUploader);
	geo->SimMeshConstraintsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshConstraints.data(), simMeshConstraintBufferByteSize, geo->SimMeshConstraintsBufferUploader);
	geo->SimMeshConstraintIDsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshConstraintIDs.data(), simMeshConstraintIDsBufferByteSize, geo->SimMeshConstraintIDsBufferUploader);
	geo->SimMeshNullSolverAccumulationBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverAccumulation.data(), simMeshNullSolverAccumulationBufferByteSize, geo->SimMeshNullSolverAccumulationBufferUploader);
	geo->SimMeshNullSolverCountBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverCount.data(), simMeshTransferBufferByteSize, geo->SimMeshNullSolverCountBufferUploader);
	geo->SimMeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshTransferIndices.data(), simMeshTransferBufferByteSize, geo->SimMeshTransferBufferUploader);
	geo->MeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), meshTransferIndices.data(), meshTransferBufferByteSize, geo->MeshTransferBufferUploader);
	geo->SimMeshVertexColorBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertexColorData.data(), simMeshVertexColorBufferByteSize, geo->SimMeshVertexColorBufferUploader);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshVertexNeighbourBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshConstraintsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshConstraintIDsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->MeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshVertexColorBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	// Normal & Tangent Resources

	std::vector<Neighbours> triangleNeighbours;
	GetVertexTriangleNeighbours(simScene->mNumMeshes, segmentedIndices, triangleNeighbours);

	const int triangleCount = indices.size() / 3;
	std::vector<TangentNormals> normals(triangleCount);
	const UINT nbByteSize = (UINT)(triangleCount) * sizeof(TangentNormals);
	const UINT adjacencyBufferByteSize = (UINT)triangleNeighbours.size() * sizeof(Neighbours);
		
	ThrowIfFailed(D3DCreateBlob(adjacencyBufferByteSize, &geo->TriangleAdjacencyBufferCPU));
	CopyMemory(geo->TriangleAdjacencyBufferCPU->GetBufferPointer(), triangleNeighbours.data(), adjacencyBufferByteSize);
	geo->TriangleAdjacencyBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), triangleNeighbours.data(), adjacencyBufferByteSize, geo->TriangleAdjacencyBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->TriangleAdjacencyBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vertexBufferByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = indexBufferByteSize;

	for (UINT i = 0; i < (UINT)subsets[filename].size(); ++i)
	{
		SubmeshGeometry submesh;
		submesh.Name = subsets[filename][i]->Name;
		submesh.VertexCount = (UINT)subsets[filename][i]->VertexCount;
		submesh.IndexCount = (UINT)subsets[filename][i]->IndexCount;
		submesh.TriangleCount = (UINT)subsets[filename][i]->TriangleCount;
		submesh.IndexStart = (UINT)subsets[filename][i]->IndexStart;
		submesh.VertexStart = (UINT)subsets[filename][i]->VertexStart;
		submesh.TriangleStart = (UINT)subsets[filename][i]->TriangleStart;
		submesh.BlendshapeVertexCount = (UINT)subsets[filename][i]->BlendshapeVertexCount;
		submesh.BlendshapeVertexStart = (UINT)subsets[filename][i]->BlendshapeVertexStart;
		submesh.BlendshapeSubsets = subsets[filename][i]->BlendshapeSubsets;
		submesh.BlendshapeCount = (UINT)subsets[filename][i]->BlendshapeCount;
		submesh.BlendshapeStart = (UINT)subsets[filename][i]->BlendshapeStart;
		submesh.SimMeshVertexCount = (UINT)subsets[filename][i]->SimMeshVertexCount;
		submesh.SimMeshIndexCount = (UINT)subsets[filename][i]->SimMeshIndexCount;
		submesh.SimMeshConstraintCount = (UINT)subsets[filename][i]->SimMeshConstraintCount;
		submesh.SimMeshConstraintStart = (UINT)subsets[filename][i]->SimMeshConstraintStart;
		submesh.SimMeshTriangleCount = (UINT)subsets[filename][i]->SimMeshTriangleCount;
		submesh.SimMeshIndexStart = (UINT)subsets[filename][i]->SimMeshIndexStart;
		submesh.SimMeshVertexStart = (UINT)subsets[filename][i]->SimMeshVertexStart;
		submesh.SimMeshTriangleStart = (UINT)subsets[filename][i]->SimMeshTriangleStart;
		geo->DrawArgs[submesh.Name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}