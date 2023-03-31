#include "../EngineApp.h"

void EngineApp::BuildMesh()
{
	std::vector<ModelLoader::SkinnedVertex> vertices;
	std::vector<std::uint16_t> indices;

	ModelLoader modelLoader;
	modelLoader.LoadModel(mSkinnedModelFilename, vertices, indices, mSkinnedSubsets, mSkinnedMats, mSkinnedInfo);

	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
	mSkinnedModelInst->ClipName = "mixamo.com";
	mSkinnedModelInst->TimePos = 0.0f;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(ModelLoader::SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = mSkinnedModelFilename;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(ModelLoader::SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	for (UINT i = 0; i < (UINT)mSkinnedSubsets.size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);

		submesh.IndexCount = (UINT)mSkinnedSubsets[i].IndexCount;
		submesh.StartIndexLocation = mSkinnedSubsets[i].IndexStart;
		submesh.BaseVertexLocation = mSkinnedSubsets[i].VertexStart;
		submesh.MaterialIndex = mSkinnedSubsets[i].MaterialIndex;
		geo->DrawArgs[name] = submesh;
	}

	mGeometries[geo->Name] = std::move(geo);
}