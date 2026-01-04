#include "../EngineApp.h"

void EngineApp::PushGenericMesh()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();

	UINT matIndex = 0;
	std::shared_ptr<Subset> boxSubset = std::make_shared<Subset>();
	boxSubset->VertexStart = boxVertexOffset;
	boxSubset->VertexCount = (UINT)box.Vertices.size();
	boxSubset->IndexStart = boxIndexOffset;
	boxSubset->IndexCount = (UINT)box.Indices32.size();
	boxSubset->TriangleStart = boxIndexOffset / 3;
	boxSubset->TriangleCount = (UINT)box.Indices32.size() / 3;
	boxSubset->Name = "box";

	std::shared_ptr<Subset> gridSubset = std::make_shared<Subset>();
	gridSubset->VertexStart = gridVertexOffset;
	gridSubset->VertexCount = (UINT)grid.Vertices.size();
	gridSubset->IndexStart = gridIndexOffset;
	gridSubset->IndexCount = (UINT)grid.Indices32.size();
	gridSubset->TriangleStart = gridIndexOffset / 3;
	gridSubset->TriangleCount = (UINT)grid.Indices32.size() / 3;
	gridSubset->Name = "grid";

	std::shared_ptr<Subset> sphereSubset = std::make_shared<Subset>();
	sphereSubset->VertexStart = sphereVertexOffset;
	sphereSubset->VertexCount = (UINT)sphere.Vertices.size();
	sphereSubset->IndexStart = sphereIndexOffset;
	sphereSubset->IndexCount = (UINT)sphere.Indices32.size();
	sphereSubset->TriangleStart = sphereIndexOffset / 3;
	sphereSubset->TriangleCount = (UINT)sphere.Indices32.size() / 3;
	sphereSubset->Name = "sphere";

	std::shared_ptr<Subset> cylinderSubset = std::make_shared<Subset>();
	cylinderSubset->VertexStart = cylinderVertexOffset;
	cylinderSubset->VertexCount = (UINT)cylinder.Vertices.size();
	cylinderSubset->IndexStart = cylinderIndexOffset;
	cylinderSubset->IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubset->TriangleStart = cylinderIndexOffset / 3;
	cylinderSubset->TriangleCount = (UINT)cylinder.Indices32.size() / 3;
	cylinderSubset->Name = "cylinder";

	std::shared_ptr<Subset> quadSubset = std::make_shared<Subset>();
	quadSubset->VertexStart = quadVertexOffset;
	quadSubset->VertexCount = (UINT)quad.Vertices.size();
	quadSubset->IndexStart = quadIndexOffset;
	quadSubset->IndexCount = (UINT)quad.Indices32.size();
	quadSubset->TriangleStart = quadIndexOffset / 3;
	quadSubset->TriangleCount = (UINT)quad.Indices32.size() / 3;
	quadSubset->Name = "quad";

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = boxSubset->IndexCount;
	boxSubmesh.IndexStart = boxSubset->IndexStart;
	boxSubmesh.VertexCount = boxSubset->VertexCount;
	boxSubmesh.VertexStart = boxSubset->VertexStart;
	boxSubmesh.TriangleCount = boxSubset->TriangleCount;
	boxSubmesh.TriangleStart = boxSubset->TriangleStart;
	boxSubmesh.Name = boxSubset->Name;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = gridSubset->IndexCount;
	gridSubmesh.IndexStart = gridSubset->IndexStart;
	gridSubmesh.VertexCount = gridSubset->VertexCount;
	gridSubmesh.VertexStart = gridSubset->VertexStart;
	gridSubmesh.TriangleCount = gridSubset->TriangleCount;
	gridSubmesh.TriangleStart = gridSubset->TriangleStart;
	gridSubmesh.Name = gridSubset->Name;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = sphereSubset->IndexCount;
	sphereSubmesh.IndexStart = sphereSubset->IndexStart;
	sphereSubmesh.VertexCount = sphereSubset->VertexCount;
	sphereSubmesh.VertexStart = sphereSubset->VertexStart;
	sphereSubmesh.TriangleCount = sphereSubset->TriangleCount;
	sphereSubmesh.TriangleStart = sphereSubset->TriangleStart;
	sphereSubmesh.Name = sphereSubset->Name;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = cylinderSubset->IndexCount;
	cylinderSubmesh.IndexStart = cylinderSubset->IndexStart;
	cylinderSubmesh.VertexCount = cylinderSubset->VertexCount;
	cylinderSubmesh.VertexStart = cylinderSubset->VertexStart;
	cylinderSubmesh.TriangleCount = cylinderSubset->TriangleCount;
	cylinderSubmesh.TriangleStart = cylinderSubset->TriangleStart;
	cylinderSubmesh.Name = cylinderSubset->Name;

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = quadSubset->IndexCount;
	quadSubmesh.IndexStart = quadSubset->IndexStart;
	quadSubmesh.VertexCount = quadSubset->VertexCount;
	quadSubmesh.VertexStart = quadSubset->VertexStart;
	quadSubmesh.TriangleCount = quadSubset->TriangleCount;
	quadSubmesh.TriangleStart = quadSubset->TriangleStart;
	quadSubmesh.Name = quadSubset->Name;

	mSubsets["shapeGeo"].push_back(std::move(boxSubset));
	mSubsets["shapeGeo"].push_back(std::move(gridSubset));
	mSubsets["shapeGeo"].push_back(std::move(sphereSubset));
	mSubsets["shapeGeo"].push_back(std::move(cylinderSubset));
	mSubsets["shapeGeo"].push_back(std::move(quadSubset));


	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(box.Vertices[i].TangentU.x, box.Vertices[i].TangentU.y, box.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(grid.Vertices[i].TangentU.x, grid.Vertices[i].TangentU.y, grid.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(sphere.Vertices[i].TangentU.x, sphere.Vertices[i].TangentU.y, sphere.Vertices[i].TangentU.z, 1.0);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(cylinder.Vertices[i].TangentU.x, cylinder.Vertices[i].TangentU.y, cylinder.Vertices[i].TangentU.z, 1.0);
	}

	for (int i = 0; i < quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = quad.Vertices[i].Position;
		vertices[k].Normal = quad.Vertices[i].Normal;
		vertices[k].TexC = quad.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(quad.Vertices[i].TangentU.x, quad.Vertices[i].TangentU.y, quad.Vertices[i].TangentU.z, 1.0);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}