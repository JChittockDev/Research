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

		sb->Id = i;
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
		sb->MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		sb->MeshName = mesh;
		sb->alias = scene->mMeshes[i]->mName.C_Str();

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

void GetTriangleNeighbors(unsigned int numMesh, std::vector<std::vector<UINT>>& segmentedIndices, std::vector<std::vector<UINT>>& segmentedTriangleNeighbours)
{
	for (UINT x = 0; x < numMesh; ++x)
	{
		std::vector<Edge> edges;
		UINT numTris = segmentedIndices[x].size() / 3;

		for (UINT i = 0; i < numTris; i++) {
			for (UINT j = 0; j < 3; j++) {
				UINT id0 = segmentedIndices[x][3 * i + j];
				UINT id1 = segmentedIndices[x][3 * i + (j + 1) % 3];
				Edge edge(Math::Min(id0, id1), Math::Max(id0, id1), 3 * i + j);
			}
		}

		std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
			if (a.id0 < b.id0)
				return true;
			if (a.id0 == b.id0 && a.id1 < b.id1)
				return true;
			return false;
			});

		std::vector<UINT> neighbors(3 * numTris, -1.0f);

		int nr = 0;
		while (nr < edges.size()) {
			Edge e0 = edges[nr];
			nr++;
			if (nr < edges.size()) {
				Edge e1 = edges[nr];
				if (e0.id0 == e1.id0 && e0.id1 == e1.id1) {
					neighbors[e0.edgeNr] = e1.edgeNr;
					neighbors[e1.edgeNr] = e0.edgeNr;
				}
				nr++;
			}
		}

		segmentedTriangleNeighbours.push_back(neighbors);
	}
}

void GetConstraintIDs(unsigned int numMesh, std::vector<std::vector<UINT>>& segmentedEdgeIds, std::vector<std::vector<UINT>>& segmentedTriPairIds, 
					std::vector<std::vector<UINT>>& segmentedIndices)
{
	std::vector<std::vector<UINT>> segmentedTriangleNeighbours;

	GetTriangleNeighbors(numMesh, segmentedIndices, segmentedTriangleNeighbours);

	for (UINT x = 0; x < numMesh; ++x)
	{
		UINT numTris = segmentedIndices[x].size() / 3;
		std::vector<UINT> edgeIds;
		std::vector<UINT> triPairIds;

		for (UINT i = 0; i < numTris; i++) {
			for (UINT j = 0; j < 3; j++) {
				UINT id0 = segmentedIndices[x][3 * i + j];
				UINT id1 = segmentedIndices[x][3 * i + (j + 1) % 3];

				// each edge only once
				UINT n = segmentedTriangleNeighbours[x][3 * i + j];
				if (n < 0 || id0 < id1) {
					edgeIds.push_back(id0);
					edgeIds.push_back(id1);
				}

				// tri pair
				if (n >= 0) {
					// opposite ids
					UINT ni = n / 3;
					UINT nj = n % 3;
					UINT id2 = segmentedIndices[x][3 * i + (j + 2) % 3];
					UINT id3 = segmentedIndices[x][3 * ni + (nj + 2) % 3];
					triPairIds.push_back(id0);
					triPairIds.push_back(id1);
					triPairIds.push_back(id2);
					triPairIds.push_back(id3);
				}
			}
		}

		segmentedEdgeIds.push_back(edgeIds);
		segmentedTriPairIds.push_back(triPairIds);
	}
}

void GetSimulationLengths(unsigned int numMesh,
	std::vector<std::shared_ptr<Subset>>& subsets,
	std::vector<std::vector<UINT>>& segmentedEdgeIds, 
	std::vector<std::vector<UINT>>& segmentedTriPairIds,
	std::vector<std::vector<UINT>>& segmentedIndices, 
	std::vector<std::vector<Vertex>>& segmentedVertices, 
	std::vector<std::vector<float>>& segmentedStretchConstraints, 
	std::vector<std::vector<float>>& segmentedBendingConstraints,
	std::vector<float>& stretchConstraints,
	std::vector<float>& bendingConstraints,
	std::vector<UINT>& stretchConstraintIDs,
	std::vector<UINT>& bendingConstraintIDs)
{
	segmentedStretchConstraints.resize(numMesh);
	segmentedBendingConstraints.resize(numMesh);

	UINT stretchConstraintCounter = 0;
	UINT bendingConstraintCounter = 0;
	for (UINT x = 0; x < numMesh; ++x)
	{
		UINT numStretchConstraintIDs = segmentedEdgeIds[x].size();
		UINT numStretchConstraints = numStretchConstraintIDs / 2;
		UINT numBendingConstraintIDs = segmentedTriPairIds[x].size();
		UINT numBendingConstraints = segmentedTriPairIds[x].size() / 4;

		segmentedStretchConstraints[x].resize(numStretchConstraints, 0);
		segmentedBendingConstraints[x].resize(numBendingConstraints, 0);

		subsets[x]->SimMeshStretchConstraintCount = numStretchConstraints;
		subsets[x]->SimMeshStretchConstraintStart = stretchConstraintCounter;
		subsets[x]->SimMeshBendingConstraintCount = numBendingConstraints;
		subsets[x]->SimMeshBendingConstraintStart = bendingConstraintCounter;
		stretchConstraintCounter += numStretchConstraints;
		bendingConstraintCounter += numBendingConstraints;
	}

	for (UINT x = 0; x < numMesh; ++x)
	{
		UINT numStretchConstraintIDs = segmentedEdgeIds[x].size();
		UINT numStretchConstraints = numStretchConstraintIDs / 2;
		UINT numBendingConstraintIDs = segmentedTriPairIds[x].size();
		UINT numBendingConstraints = segmentedTriPairIds[x].size() / 4;

		for (UINT i = 0; i < numStretchConstraints; i++) {
			UINT id0 = segmentedEdgeIds[x][2 * i];
			UINT id1 = segmentedEdgeIds[x][2 * i + 1];
			segmentedStretchConstraints[x][i] = Math::Length(segmentedVertices[x][id0].Pos, segmentedVertices[x][id1].Pos);
		}

		for (UINT i = 0; i < numBendingConstraints; i++) {
			UINT id0 = segmentedTriPairIds[x][4 * i + 2];
			UINT id1 = segmentedTriPairIds[x][4 * i + 3];
			segmentedBendingConstraints[x][i] = Math::Length(segmentedVertices[x][id0].Pos, segmentedVertices[x][id1].Pos);
		}
	}

	for (UINT x = 0; x < numMesh; ++x)
	{
		UINT numStretchConstraintIDs = segmentedEdgeIds[x].size();
		UINT numStretchConstraints = numStretchConstraintIDs / 2;
		UINT numBendingConstraintIDs = segmentedTriPairIds[x].size();
		UINT numBendingConstraints = segmentedTriPairIds[x].size() / 4;

		for (UINT i = 0; i < numStretchConstraints; i++) {
			stretchConstraints.push_back(segmentedStretchConstraints[x][i]);
		}

		for (UINT i = 0; i < numBendingConstraints; i++) {
			bendingConstraints.push_back(segmentedBendingConstraints[x][i]);
		}

		for (UINT i = 0; i < numStretchConstraintIDs; i++) {
			stretchConstraintIDs.push_back(segmentedEdgeIds[x][i]);
		}

		for (UINT i = 0; i < numBendingConstraintIDs; i++) {
			bendingConstraintIDs.push_back(segmentedTriPairIds[x][i]);
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
	std::vector<BlendshapeVertex> blendshapes;

	std::vector<std::vector<UINT>> segmentedIndices;
	std::vector<std::vector<Vertex>> segmentedVertices;

	Assimp::Importer imp;
	const aiScene* scene = imp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	std::shared_ptr<Skeleton> mSkeleton = std::make_shared<Skeleton>();
	mSkeleton->rootNodeName = scene->mRootNode->mName.data;
	ReadSkeleton(scene, mSkeleton);
	ReadMaterials(scene, mats);
	ReadVertices(scene->mNumMeshes, scene->mMeshes, vertices, segmentedVertices);
	ReadBlendshapeData(scene->mNumMeshes, scene->mMeshes, blendshapes);
	ReadSkinningData(scene->mNumMeshes, scene->mMeshes, mSkeleton, vertices, skinning);
	ReadTriangles(scene->mNumMeshes, scene->mMeshes, indices, segmentedIndices);
	ReadAnimations(scene, animations);
	ReadTransformNodes(scene->mRootNode, transforms);
	LinkTransformNodes(scene->mRootNode, transforms);
	skeletons[filename] = std::move(mSkeleton);

	const UINT vertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT skinningBufferByteSize = (UINT)skinning.size() * sizeof(SkinningInfo);
	const UINT indexBufferByteSize = (UINT)indices.size() * sizeof(UINT);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = filename;
	geo->Simulation = true;

	ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(skinningBufferByteSize, &geo->SkinningBufferCPU));
	CopyMemory(geo->SkinningBufferCPU->GetBufferPointer(), skinning.data(), skinningBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->SkinnedVertexBufferCPU));
	CopyMemory(geo->SkinnedVertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(indexBufferByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), indexBufferByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->VertexBufferUploader);
	geo->SkinningBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), skinning.data(), skinningBufferByteSize, geo->SkinningBufferUploader);
	geo->SkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->SkinnedVertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), indexBufferByteSize, geo->IndexBufferUploader);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinningBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	if (blendshapes.size() > 0)
	{
		const UINT blendshapeBufferByteSize = (UINT)blendshapes.size() * sizeof(BlendshapeVertex);

		ThrowIfFailed(D3DCreateBlob(blendshapeBufferByteSize, &geo->BlendshapeBufferCPU));
		CopyMemory(geo->BlendshapeBufferCPU->GetBufferPointer(), blendshapes.data(), blendshapeBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->BlendedVertexBufferCPU));
		CopyMemory(geo->BlendedVertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

		geo->BlendshapeBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), blendshapes.data(), blendshapeBufferByteSize, geo->BlendshapeBufferUploader);
		geo->BlendedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->BlendedVertexBufferUploader);
		
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->BlendshapeBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}

	if (geo->Simulation)
	{
		Assimp::Importer simImp;
		simImp.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS | aiComponent_TEXCOORDS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS | aiComponent_TEXTURES | aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_MATERIALS);
		const aiScene* simScene = simImp.ReadFile(filename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveComponent);

		// Simulation Resources

		std::vector<Vertex> simMeshVertices;
		std::vector<UINT> simMeshIndices;
		std::vector<std::vector<Vertex>> simMeshSegmentedVertices;
		std::vector<std::vector<UINT>> simMeshSegmentedIndices;
		std::vector<std::vector<UINT>> simMeshSegmentedEdgeIds;
		std::vector<std::vector<UINT>> simMeshSegmentedTriPairIds;
		std::vector<std::vector<float>> simMeshSegmentedStretchConstraints;
		std::vector<std::vector<float>> simMeshSegmentedBendingConstraints;
		std::vector<float> simMeshStretchConstraints;
		std::vector<float> simMeshBendingConstraints;
		std::vector<UINT> simMeshStretchConstraintIDs;
		std::vector<UINT> simMeshBendingConstraintIDs;
		std::vector<UINT> meshTransferIndices;
		std::vector<UINT> simMeshTransferIndices;

		ReadVertices(simScene->mNumMeshes, simScene->mMeshes, simMeshVertices, simMeshSegmentedVertices);
		ReadTriangles(simScene->mNumMeshes, simScene->mMeshes, simMeshIndices, simMeshSegmentedIndices);
		ReadSubsetTable(scene, simScene, subsets, filename);

		GetMeshTransferMap(segmentedVertices, simMeshSegmentedVertices, meshTransferIndices);
		GetMeshTransferMap(simMeshSegmentedVertices, segmentedVertices, simMeshTransferIndices);
		GetConstraintIDs(simScene->mNumMeshes, simMeshSegmentedEdgeIds, simMeshSegmentedTriPairIds, simMeshSegmentedIndices);
		GetSimulationLengths(simScene->mNumMeshes, subsets[filename], simMeshSegmentedEdgeIds, simMeshSegmentedTriPairIds, simMeshSegmentedIndices, simMeshSegmentedVertices,
		simMeshSegmentedStretchConstraints, simMeshSegmentedBendingConstraints, simMeshStretchConstraints, simMeshBendingConstraints, simMeshStretchConstraintIDs, simMeshBendingConstraintIDs);

		const UINT simMeshVertexBufferByteSize = (UINT)simMeshVertices.size() * sizeof(Vertex);
		const UINT simMeshForceBufferByteSize = (UINT)simMeshVertices.size() * sizeof(Vector3);
		const UINT simMeshTransferBufferByteSize = (UINT)simMeshVertices.size() * sizeof(UINT);
		const UINT simMeshStretchConstraintBufferByteSize = (UINT)simMeshStretchConstraints.size() * sizeof(float);
		const UINT simMeshBendingConstraintBufferByteSize = (UINT)simMeshBendingConstraints.size() * sizeof(float);
		const UINT simMeshStretchConstraintIDsBufferByteSize = (UINT)simMeshStretchConstraintIDs.size() * sizeof(UINT);
		const UINT simMeshBendingConstraintIDsBufferByteSize = (UINT)simMeshBendingConstraintIDs.size() * sizeof(UINT);
		const UINT meshTransferBufferByteSize = (UINT)vertices.size() * sizeof(UINT);

		std::vector<Vector3> simMeshForce(simMeshVertices.size());
		std::vector<Vector3> simMeshSolverAccumulation(simMeshVertices.size());
		std::vector<UINT> simMeshSolverCount(simMeshVertices.size());

		ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshSkinnedVertexBufferCPU));
		CopyMemory(geo->SimMeshSkinnedVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshPreviousSkinnedVertexBufferCPU));
		CopyMemory(geo->SimMeshPreviousSkinnedVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshSolverVertexBufferCPU));
		CopyMemory(geo->SimMeshSolverVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshForceBufferByteSize, &geo->SimMeshForceBufferCPU));
		CopyMemory(geo->SimMeshForceBufferCPU->GetBufferPointer(), simMeshForce.data(), simMeshForceBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshStretchConstraintsVertexBufferCPU));
		CopyMemory(geo->SimMeshStretchConstraintsVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);
		
		ThrowIfFailed(D3DCreateBlob(simMeshVertexBufferByteSize, &geo->SimMeshBendingConstraintsVertexBufferCPU));
		CopyMemory(geo->SimMeshBendingConstraintsVertexBufferCPU->GetBufferPointer(), simMeshVertices.data(), simMeshVertexBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshStretchConstraintBufferByteSize, &geo->SimMeshStretchConstraintsBufferCPU));
		CopyMemory(geo->SimMeshStretchConstraintsBufferCPU->GetBufferPointer(), simMeshStretchConstraints.data(), simMeshStretchConstraintBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshBendingConstraintBufferByteSize, &geo->SimMeshBendingConstraintsBufferCPU));
		CopyMemory(geo->SimMeshBendingConstraintsBufferCPU->GetBufferPointer(), simMeshBendingConstraints.data(), simMeshBendingConstraintBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshStretchConstraintIDsBufferByteSize, &geo->SimMeshStretchConstraintIDsBufferCPU));
		CopyMemory(geo->SimMeshStretchConstraintIDsBufferCPU->GetBufferPointer(), simMeshStretchConstraintIDs.data(), simMeshStretchConstraintIDsBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshBendingConstraintIDsBufferByteSize, &geo->SimMeshBendingConstraintIDsBufferCPU));
		CopyMemory(geo->SimMeshBendingConstraintIDsBufferCPU->GetBufferPointer(), simMeshBendingConstraintIDs.data(), simMeshBendingConstraintIDsBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshForceBufferByteSize, &geo->SimMeshSolverAccumulationBufferCPU));
		CopyMemory(geo->SimMeshSolverAccumulationBufferCPU->GetBufferPointer(), simMeshSolverAccumulation.data(), simMeshForceBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshTransferBufferByteSize, &geo->SimMeshSolverCountBufferCPU));
		CopyMemory(geo->SimMeshSolverCountBufferCPU->GetBufferPointer(), simMeshSolverCount.data(), simMeshTransferBufferByteSize); 

		ThrowIfFailed(D3DCreateBlob(simMeshForceBufferByteSize, &geo->SimMeshNullSolverAccumulationBufferCPU));
		CopyMemory(geo->SimMeshNullSolverAccumulationBufferCPU->GetBufferPointer(), simMeshSolverAccumulation.data(), simMeshForceBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshTransferBufferByteSize, &geo->SimMeshNullSolverCountBufferCPU));
		CopyMemory(geo->SimMeshNullSolverCountBufferCPU->GetBufferPointer(), simMeshSolverCount.data(), simMeshTransferBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(simMeshTransferBufferByteSize, &geo->SimMeshTransferBufferCPU));
		CopyMemory(geo->SimMeshTransferBufferCPU->GetBufferPointer(), simMeshTransferIndices.data(), simMeshTransferBufferByteSize);

		ThrowIfFailed(D3DCreateBlob(meshTransferBufferByteSize, &geo->MeshTransferBufferCPU));
		CopyMemory(geo->MeshTransferBufferCPU->GetBufferPointer(), meshTransferIndices.data(), meshTransferBufferByteSize);

		geo->SimMeshSkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshSkinnedVertexBufferUploader);
		geo->SimMeshPreviousSkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshPreviousSkinnedVertexBufferUploader);
		geo->SimMeshSolverVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshSolverVertexBufferUploader);
		geo->SimMeshForceBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshForce.data(), simMeshForceBufferByteSize, geo->SimMeshForceBufferUploader);
		geo->SimMeshStretchConstraintsVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshStretchConstraintsVertexBufferUploader);
		geo->SimMeshBendingConstraintsVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshVertices.data(), simMeshVertexBufferByteSize, geo->SimMeshBendingConstraintsVertexBufferUploader);
		geo->SimMeshStretchConstraintsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshStretchConstraints.data(), simMeshStretchConstraintBufferByteSize, geo->SimMeshStretchConstraintsBufferUploader);
		geo->SimMeshBendingConstraintsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshBendingConstraints.data(), simMeshBendingConstraintBufferByteSize, geo->SimMeshBendingConstraintsBufferUploader);
		geo->SimMeshStretchConstraintIDsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshStretchConstraintIDs.data(), simMeshStretchConstraintIDsBufferByteSize, geo->SimMeshStretchConstraintIDsBufferUploader);
		geo->SimMeshBendingConstraintIDsBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshBendingConstraintIDs.data(), simMeshBendingConstraintIDsBufferByteSize, geo->SimMeshBendingConstraintIDsBufferUploader);
		geo->SimMeshSolverAccumulationBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverAccumulation.data(), simMeshForceBufferByteSize, geo->SimMeshSolverAccumulationBufferUploader);
		geo->SimMeshSolverCountBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverCount.data(), simMeshTransferBufferByteSize, geo->SimMeshSolverCountBufferUploader);
		geo->SimMeshNullSolverAccumulationBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverAccumulation.data(), simMeshForceBufferByteSize, geo->SimMeshNullSolverAccumulationBufferUploader);
		geo->SimMeshNullSolverCountBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshSolverCount.data(), simMeshTransferBufferByteSize, geo->SimMeshNullSolverCountBufferUploader);
		geo->SimMeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), simMeshTransferIndices.data(), simMeshTransferBufferByteSize, geo->SimMeshTransferBufferUploader);
		geo->MeshTransferBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), meshTransferIndices.data(), meshTransferBufferByteSize, geo->MeshTransferBufferUploader);

		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshStretchConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshBendingConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshStretchConstraintsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshBendingConstraintsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshStretchConstraintIDsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshBendingConstraintIDsBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SimMeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->MeshTransferBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		// Normal & Tangent Resources

		std::vector<Neighbours> triangleNeighbours;
		GetVertexTriangleNeighbours(simScene->mNumMeshes, segmentedIndices, triangleNeighbours);


		const int triangleCount = indices.size() / 3;
		const UINT nbByteSize = (UINT)(triangleCount) * sizeof(TangentNormals);
		const UINT abByteSize = (UINT)triangleNeighbours.size() * sizeof(Neighbours);

		std::vector<TangentNormals> normals;
		normals.resize(triangleCount);

		ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->TransformedVertexBufferCPU));
		CopyMemory(geo->TransformedVertexBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);
		
		ThrowIfFailed(D3DCreateBlob(abByteSize, &geo->TriangleAdjacencyBufferCPU));
		CopyMemory(geo->TriangleAdjacencyBufferCPU->GetBufferPointer(), triangleNeighbours.data(), abByteSize);

		ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->TriangleNormalBufferCPU));
		CopyMemory(geo->TriangleNormalBufferCPU->GetBufferPointer(), normals.data(), nbByteSize);

		ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexNormalBufferCPU));
		CopyMemory(geo->VertexNormalBufferCPU->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

		geo->TransformedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->TransformedVertexBufferUploader);
		geo->TriangleAdjacencyBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), triangleNeighbours.data(), abByteSize, geo->TriangleAdjacencyBufferUploader);
		geo->TriangleNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), normals.data(), nbByteSize, geo->TriangleNormalBufferUploader);
		geo->VertexNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->VertexNormalBufferUploader);
		
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->TriangleAdjacencyBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	}
	else
	{
		ReadSubsetTable(scene, subsets, filename);

		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vertexBufferByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = indexBufferByteSize;

	for (UINT i = 0; i < (UINT)subsets[filename].size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);
		
		submesh.alias = subsets[filename][i]->alias;
		submesh.VertexCount = (UINT)subsets[filename][i]->VertexCount;
		submesh.IndexCount = (UINT)subsets[filename][i]->IndexCount;
		submesh.TriangleCount = (UINT)subsets[filename][i]->TriangleCount;
		submesh.StartIndexLocation = (UINT)subsets[filename][i]->IndexStart;
		submesh.StartVertexLocation = (UINT)subsets[filename][i]->VertexStart;
		submesh.StartTriangleLocation = (UINT)subsets[filename][i]->TriangleStart;
		submesh.MaterialIndex = (UINT)subsets[filename][i]->MaterialIndex;
		submesh.BlendshapeVertexCount = (UINT)subsets[filename][i]->BlendshapeVertexCount;
		submesh.BlendshapeVertexStart = (UINT)subsets[filename][i]->BlendshapeVertexStart;
		submesh.BlendshapeSubsets = subsets[filename][i]->BlendshapeSubsets;
		submesh.BlendshapeCount = (UINT)subsets[filename][i]->BlendshapeCount;
		submesh.BlendshapeStart = (UINT)subsets[filename][i]->BlendshapeStart;
		
		if (geo->Simulation)
		{
			submesh.SimMeshVertexCount = (UINT)subsets[filename][i]->SimMeshVertexCount;
			submesh.SimMeshIndexCount = (UINT)subsets[filename][i]->SimMeshIndexCount;
			submesh.SimMeshStretchConstraintCount = (UINT)subsets[filename][i]->SimMeshStretchConstraintCount;
			submesh.SimMeshStretchConstraintStart = (UINT)subsets[filename][i]->SimMeshStretchConstraintStart;
			submesh.SimMeshBendingConstraintCount = (UINT)subsets[filename][i]->SimMeshBendingConstraintCount;
			submesh.SimMeshBendingConstraintStart = (UINT)subsets[filename][i]->SimMeshBendingConstraintStart;
			submesh.SimMeshTriangleCount = (UINT)subsets[filename][i]->SimMeshTriangleCount;
			submesh.SimMeshStartIndexLocation = (UINT)subsets[filename][i]->SimMeshIndexStart;
			submesh.SimMeshStartVertexLocation = (UINT)subsets[filename][i]->SimMeshVertexStart;
			submesh.SimMeshStartTriangleLocation = (UINT)subsets[filename][i]->SimMeshTriangleStart;
		}

		geo->DrawArgs[name] = submesh;
	}

	geometries[geo->Name] = std::move(geo);
}