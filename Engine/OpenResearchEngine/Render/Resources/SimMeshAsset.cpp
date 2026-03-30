#include "SimMeshAsset.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/D3Dx12.h"

// ──────────────────────────────────────────────────────────────────────────────
// Load() body: port the sim-path re-import from Mesh.cpp's animated constructor.
//
// The sim import uses:
//   aiProcess_RemoveComponent with flags stripping normals, tangents, colors,
//   bone weights, and animations (see Mesh.cpp for the exact aiProcess flags).
//   aiProcess_JoinIdenticalVertices to weld vertices for physics.
//
// After import, call the free functions in Mesh.cpp:
//   GetMeshTransferMap()           -> SimMeshTransferBufferGPU, MeshTransferBufferGPU
//   GetConstraints()               -> SimMeshConstraintsBufferGPU, SimMeshConstraintIDsBufferGPU
//   GetVertexNeighbours()          -> SimMeshVertexNeighbourBufferGPU
//   (vertex color from .jpg file)  -> SimMeshVertexColorBufferGPU
//   (TriangleAdjacency from render DrawArgs indices) -> TriangleAdjacencyBufferGPU
//
// All buffers transition from GENERIC_READ -> NON_PIXEL_SHADER_RESOURCE.
// Null solver buffers are zero-initialized vectors, same size as sim vertex count.
//
// Populate SubsetData[subsetName] from both the sim importer output and
// renderDrawArgs (for VertexStart/VertexCount/TriangleStart/TriangleCount).
// ──────────────────────────────────────────────────────────────────────────────

std::shared_ptr<SimMeshAsset> SimMeshAsset::Load(
    const std::string& filePath,
    const std::string& vertexColorPath,
    Microsoft::WRL::ComPtr<ID3D12Device>              device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
    const std::unordered_map<std::string, SubmeshGeometry>& renderDrawArgs)
{
    auto asset = std::make_shared<SimMeshAsset>();
    // Port sim-path logic from Mesh.cpp animated constructor here.
    // All buffer transitions: GENERIC_READ -> NON_PIXEL_SHADER_RESOURCE
    return asset;
}

void SimMeshAsset::DisposeUploaders() {
    SimMeshVertexBufferUploader = nullptr;
    SimMeshConstraintsBufferUploader = nullptr;
    SimMeshConstraintIDsBufferUploader = nullptr;
    SimMeshVertexNeighbourBufferUploader = nullptr;
    SimMeshVertexColorBufferUploader = nullptr;
    SimMeshTransferBufferUploader = nullptr;
    MeshTransferBufferUploader = nullptr;
    TriangleAdjacencyBufferUploader = nullptr;
    SimMeshNullSolverAccumulationBufferUploader = nullptr;
    SimMeshNullSolverCountBufferUploader = nullptr;
}
