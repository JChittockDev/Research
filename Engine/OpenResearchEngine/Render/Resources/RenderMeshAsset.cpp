#include "RenderMeshAsset.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/D3Dx12.h"

// ──────────────────────────────────────────────────────────────────────────────
// The body of Load() replicates the render-path logic from the animated Mesh
// constructor in Render/Resources/Mesh.cpp.
//
// Steps to port (reference the animated constructor in Mesh.cpp):
//   1. Open Mesh.cpp. Find the animated constructor (takes skeletons/animations/transforms).
//   2. Copy the Assimp importer setup, aiProcess flags, and vertex extraction loop
//      that builds the std::vector<Vertex> and std::vector<uint16_t> index arrays.
//   3. Copy the SkinningInfo extraction (bone weights/indices) if bones are present.
//   4. Copy the blendshape extraction if morph targets are present.
//   5. Copy the SubmeshGeometry / DrawArgs population.
//   6. Do NOT copy any sim-mesh code (vertices that set aiProcess_RemoveComponent
//      or aiProcess_JoinIdenticalVertices for the physics re-import).
//   7. Replace buffer creation calls (d3dUtil::CreateDefaultBuffer) with the
//      pattern shown below, and add the immediate resource barrier transitions.
//
// Buffer state transitions applied immediately after CreateDefaultBuffer:
//   VertexBufferGPU:
//     forAnimation=true  → GENERIC_READ → NON_PIXEL_SHADER_RESOURCE
//     forAnimation=false → GENERIC_READ → VERTEX_AND_CONSTANT_BUFFER
//   IndexBufferGPU      → GENERIC_READ → (no transition; stays GENERIC_READ / index buffer compatible)
//   SkinningBufferGPU   → GENERIC_READ → NON_PIXEL_SHADER_RESOURCE
//   BlendshapeBufferGPU → GENERIC_READ → NON_PIXEL_SHADER_RESOURCE
// ──────────────────────────────────────────────────────────────────────────────

std::shared_ptr<RenderMeshAsset> RenderMeshAsset::Load(
    const std::string& filePath,
    const std::string& vertexColorPath,
    bool forAnimation,
    Microsoft::WRL::ComPtr<ID3D12Device>              device,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& outSubsets)
{
    auto asset = std::make_shared<RenderMeshAsset>();

    // ── Port Assimp render-path import from Mesh.cpp animated constructor ──
    // (vertices, indices, skinning, blendshapes, DrawArgs population)
    // Replace all MeshGeometry buffer assignments with asset->XxxBufferGPU.
    // Example pattern for one buffer (repeat for each):

    // std::vector<Vertex> vertices = /* ... from Assimp ... */;
    // UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    // asset->VertexByteStride     = sizeof(Vertex);
    // asset->VertexBufferByteSize = vbByteSize;
    // asset->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
    //     device.Get(), cmdList.Get(), vertices.data(), vbByteSize,
    //     asset->VertexBufferUploader);
    //
    // D3D12_RESOURCE_STATES targetState = forAnimation
    //     ? D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
    //     : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    // cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //     asset->VertexBufferGPU.Get(),
    //     D3D12_RESOURCE_STATE_GENERIC_READ, targetState));
    //
    // // Skinning (if bones found):
    // asset->hasSkinning = true;
    // asset->SkinningBufferGPU = d3dUtil::CreateDefaultBuffer(
    //     device.Get(), cmdList.Get(), skinningData.data(), skinByteSize,
    //     asset->SkinningBufferUploader);
    // cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //     asset->SkinningBufferGPU.Get(),
    //     D3D12_RESOURCE_STATE_GENERIC_READ,
    //     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    return asset;
}

D3D12_VERTEX_BUFFER_VIEW RenderMeshAsset::VertexBufferView() const {
    return { VertexBufferGPU->GetGPUVirtualAddress(), VertexBufferByteSize, VertexByteStride };
}

D3D12_INDEX_BUFFER_VIEW RenderMeshAsset::IndexBufferView() const {
    return { IndexBufferGPU->GetGPUVirtualAddress(), IndexBufferByteSize, IndexFormat };
}

D3D12_VERTEX_BUFFER_VIEW RenderMeshAsset::MakeVertexBufferView(ID3D12Resource* vb) const {
    return { vb->GetGPUVirtualAddress(), VertexBufferByteSize, VertexByteStride };
}

void RenderMeshAsset::DisposeUploaders() {
    VertexBufferUploader   = nullptr;
    IndexBufferUploader    = nullptr;
    SkinningBufferUploader = nullptr;
    BlendshapeBufferUploader = nullptr;
}
