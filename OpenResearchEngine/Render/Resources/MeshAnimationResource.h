#pragma once

#include "Skeleton.h"

class MeshAnimationResource
{
public:
    MeshAnimationResource::MeshAnimationResource(Microsoft::WRL::ComPtr<ID3D12Device>& md3dDevice, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, MeshDeformationData& deformationData);

public:

    // Default Buffers
    Microsoft::WRL::ComPtr<ID3DBlob> SkinnedVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> BlendedVertexBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SkinnedVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendedVertexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SkinnedVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendedVertexBufferUploader = nullptr;

    // Simulation Buffers
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshSkinnedVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshConstraintsVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshSolverAccumulationBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshSolverCountBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshSolverVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshPreviousSkinnedVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> SimMeshForceBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSkinnedVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverAccumulationBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverCountBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshPreviousSkinnedVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshForceBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSkinnedVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverAccumulationBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverCountBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshPreviousSkinnedVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshForceBufferUploader = nullptr;

    // Normal & Tangent Calculation Buffers
    Microsoft::WRL::ComPtr<ID3DBlob> TransformedVertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> TriangleNormalBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> VertexNormalBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> TransformedVertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleNormalBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexNormalBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> TransformedVertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleNormalBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexNormalBufferUploader = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;

    D3D12_VERTEX_BUFFER_VIEW SkinnedVertexBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW svbv;
        svbv.BufferLocation = SkinnedVertexBufferGPU->GetGPUVirtualAddress();
        svbv.StrideInBytes = VertexByteStride;
        svbv.SizeInBytes = VertexBufferByteSize;
        return svbv;
    }

    D3D12_VERTEX_BUFFER_VIEW VertexNormalBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW svbv;
        svbv.BufferLocation = VertexNormalBufferGPU->GetGPUVirtualAddress();
        svbv.StrideInBytes = VertexByteStride;
        svbv.SizeInBytes = VertexBufferByteSize;
        return svbv;
    }

    // We can free this memory after we finish upload to the GPU.
    void DisposeUploaders()
    {
        SkinnedVertexBufferUploader = nullptr;
        BlendedVertexBufferUploader = nullptr;
        SimMeshSkinnedVertexBufferUploader = nullptr;
        SimMeshPreviousSkinnedVertexBufferUploader = nullptr;
        SimMeshForceBufferUploader = nullptr;
        SimMeshSolverVertexBufferUploader = nullptr;
        SimMeshConstraintsVertexBufferUploader = nullptr;
        SimMeshSolverAccumulationBufferUploader = nullptr;
        SimMeshSolverCountBufferUploader = nullptr;
        TransformedVertexBufferUploader = nullptr;
        TriangleNormalBufferUploader = nullptr;
        VertexNormalBufferUploader = nullptr;
    }
};