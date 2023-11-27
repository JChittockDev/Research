#include "MeshAnimationResource.h"

MeshAnimationResource::MeshAnimationResource(Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, MeshDeformationData& deformationData)
{
	const UINT skinningVertexBufferByteSize = (UINT)deformationData.vertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(skinningVertexBufferByteSize, &SkinnedVertexBufferCPU));
	CopyMemory(SkinnedVertexBufferCPU->GetBufferPointer(), deformationData.vertices.data(), skinningVertexBufferByteSize);
	SkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.vertices.data(), skinningVertexBufferByteSize, SkinnedVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT blendshapeVertexBufferByteSize = (UINT)deformationData.vertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(blendshapeVertexBufferByteSize, &BlendedVertexBufferCPU));
	CopyMemory(BlendedVertexBufferCPU->GetBufferPointer(), deformationData.vertices.data(), blendshapeVertexBufferByteSize);
	BlendedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.vertices.data(), blendshapeVertexBufferByteSize, BlendedVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshSkinnedVertexBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(simMeshSkinnedVertexBufferByteSize, &SimMeshSkinnedVertexBufferCPU));
	CopyMemory(SimMeshSkinnedVertexBufferCPU->GetBufferPointer(), deformationData.simMeshVertices.data(), simMeshSkinnedVertexBufferByteSize);
	SimMeshSkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshVertices.data(), simMeshSkinnedVertexBufferByteSize, SimMeshSkinnedVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshPreviousSkinnedVertexBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(simMeshPreviousSkinnedVertexBufferByteSize, &SimMeshPreviousSkinnedVertexBufferCPU));
	CopyMemory(SimMeshPreviousSkinnedVertexBufferCPU->GetBufferPointer(), deformationData.simMeshVertices.data(), simMeshPreviousSkinnedVertexBufferByteSize);
	SimMeshPreviousSkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshVertices.data(), simMeshPreviousSkinnedVertexBufferByteSize, SimMeshPreviousSkinnedVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshSolverVertexBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(simMeshSolverVertexBufferByteSize, &SimMeshSolverVertexBufferCPU));
	CopyMemory(SimMeshSolverVertexBufferCPU->GetBufferPointer(), deformationData.simMeshVertices.data(), simMeshSolverVertexBufferByteSize);
	SimMeshSolverVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshVertices.data(), simMeshSolverVertexBufferByteSize, SimMeshSolverVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshForceBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vector3);
	ThrowIfFailed(D3DCreateBlob(simMeshForceBufferByteSize, &SimMeshForceBufferCPU));
	CopyMemory(SimMeshForceBufferCPU->GetBufferPointer(), deformationData.simMeshForce.data(), simMeshForceBufferByteSize);
	SimMeshForceBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshForce.data(), simMeshForceBufferByteSize, SimMeshForceBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));


	const UINT simMeshStretchConstraintsVertexBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(simMeshStretchConstraintsVertexBufferByteSize, &SimMeshStretchConstraintsVertexBufferCPU));
	CopyMemory(SimMeshStretchConstraintsVertexBufferCPU->GetBufferPointer(), deformationData.simMeshVertices.data(), simMeshStretchConstraintsVertexBufferByteSize);
	SimMeshStretchConstraintsVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshVertices.data(), simMeshStretchConstraintsVertexBufferByteSize, SimMeshStretchConstraintsVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshStretchConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshConstraintsVertexBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(simMeshConstraintsVertexBufferByteSize, &SimMeshConstraintsVertexBufferCPU));
	CopyMemory(SimMeshConstraintsVertexBufferCPU->GetBufferPointer(), deformationData.simMeshVertices.data(), simMeshConstraintsVertexBufferByteSize);
	SimMeshConstraintsVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshVertices.data(), simMeshConstraintsVertexBufferByteSize, SimMeshConstraintsVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshAccumulationBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(Vector3);
	ThrowIfFailed(D3DCreateBlob(simMeshAccumulationBufferByteSize, &SimMeshSolverAccumulationBufferCPU));
	CopyMemory(SimMeshSolverAccumulationBufferCPU->GetBufferPointer(), deformationData.simMeshSolverAccumulation.data(), simMeshAccumulationBufferByteSize);
	SimMeshSolverAccumulationBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshSolverAccumulation.data(), simMeshAccumulationBufferByteSize, SimMeshSolverAccumulationBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT simMeshSolverCountBufferByteSize = (UINT)deformationData.simMeshVertices.size() * sizeof(UINT);
	ThrowIfFailed(D3DCreateBlob(simMeshSolverCountBufferByteSize, &SimMeshSolverCountBufferCPU));
	CopyMemory(SimMeshSolverCountBufferCPU->GetBufferPointer(), deformationData.simMeshSolverCount.data(), simMeshSolverCountBufferByteSize);
	SimMeshSolverCountBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.simMeshSolverCount.data(), simMeshSolverCountBufferByteSize, SimMeshSolverCountBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT normalBufferByteSize = (UINT)deformationData.normals.size() * sizeof(TangentNormals);
	ThrowIfFailed(D3DCreateBlob(normalBufferByteSize, &TriangleNormalBufferCPU));
	CopyMemory(TriangleNormalBufferCPU->GetBufferPointer(), deformationData.normals.data(), normalBufferByteSize);
	TriangleNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.normals.data(), normalBufferByteSize, TriangleNormalBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT transformedVertexBufferByteSize = (UINT)deformationData.vertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(transformedVertexBufferByteSize, &TransformedVertexBufferCPU));
	CopyMemory(TransformedVertexBufferCPU->GetBufferPointer(), deformationData.vertices.data(), transformedVertexBufferByteSize);
	TransformedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.vertices.data(), transformedVertexBufferByteSize, TransformedVertexBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	const UINT vertexNormalBufferByteSize = (UINT)deformationData.vertices.size() * sizeof(Vertex);
	ThrowIfFailed(D3DCreateBlob(vertexNormalBufferByteSize, &VertexNormalBufferCPU));
	CopyMemory(VertexNormalBufferCPU->GetBufferPointer(), deformationData.vertices.data(), vertexNormalBufferByteSize);
	VertexNormalBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), deformationData.vertices.data(), vertexNormalBufferByteSize, VertexNormalBufferUploader);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	VertexByteStride = sizeof(Vertex);
	VertexBufferByteSize = (UINT)deformationData.vertices.size() * sizeof(Vertex);
}