#pragma once
#include "../../Common/Structures.h"
#include "../../D3D12/D3DUtil.h"
#include <string>
#include <memory>
#include <unordered_map>

// Per-subset sim data offsets (mirrors what SubmeshGeometry has for render side)
struct SimSubset {
    UINT SimMeshVertexStart     = 0;
    UINT SimMeshVertexCount     = 0;
    UINT SimMeshIndexStart      = 0;
    UINT SimMeshIndexCount      = 0;
    UINT SimMeshConstraintStart = 0;
    UINT SimMeshConstraintCount = 0;
    UINT SimMeshTriangleStart   = 0;
    UINT SimMeshTriangleCount   = 0;
    // Render-side offsets needed by passes that bridge sim<->render:
    UINT VertexStart  = 0;
    UINT VertexCount  = 0;
    UINT TriangleStart = 0;
    UINT TriangleCount = 0;
};

// Immutable physics topology for one source file.
// Created only when ItemData::simulated == true.
// Re-imports the same FBX with aiProcess_RemoveComponent + aiProcess_JoinIdenticalVertices.
class SimMeshAsset {
public:
    static std::shared_ptr<SimMeshAsset> Load(
        const std::string& filePath,
        const std::string& vertexColorPath,
        Microsoft::WRL::ComPtr<ID3D12Device>              device,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
        const std::unordered_map<std::string, SubmeshGeometry>& renderDrawArgs
    );

    // Physics topology buffers (all NON_PIXEL_SHADER_RESOURCE after upload)
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintIDsBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexNeighbourBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexColorBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshTransferBufferGPU;   // sim->render map
    Microsoft::WRL::ComPtr<ID3D12Resource> MeshTransferBufferGPU;      // render->sim map
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleAdjacencyBufferGPU;

    // Null buffers for per-frame accumulation reset (PostSolve CopyResource source)
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverAccumulationBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverCountBufferGPU;

    // Per-subset data
    std::unordered_map<std::string, SimSubset> SubsetData;

    void DisposeUploaders();

private:
    // Uploaders (released after GPU upload completes)
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintIDsBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexNeighbourBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshVertexColorBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshTransferBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> MeshTransferBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleAdjacencyBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverAccumulationBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshNullSolverCountBufferUploader;
};
