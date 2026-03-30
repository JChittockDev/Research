#pragma once
#include "../../Common/Structures.h"
#include "../../D3D12/D3DUtil.h"
#include <string>
#include <unordered_map>
#include <memory>

// Immutable, shared GPU buffers for one FBX/OBJ source file (render path).
// Load once, referenced by multiple MeshInstance objects.
class RenderMeshAsset {
public:
    // Static factory — imports file, uploads GPU buffers, transitions resource states.
    // forAnimation=true  → VertexBufferGPU ends in NON_PIXEL_SHADER_RESOURCE (used by compute)
    // forAnimation=false → VertexBufferGPU ends in VERTEX_AND_CONSTANT_BUFFER  (direct draw)
    static std::shared_ptr<RenderMeshAsset> Load(
        const std::string& filePath,
        const std::string& vertexColorPath,
        bool forAnimation,
        Microsoft::WRL::ComPtr<ID3D12Device>               device,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>  cmdList,
        std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>& outSubsets
    );

    // Always present
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader;

    UINT        VertexByteStride     = 0;
    UINT        VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat          = DXGI_FORMAT_R16_UINT;
    UINT        IndexBufferByteSize  = 0;

    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

    // Present only when source has bone weights
    bool hasSkinning = false;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SkinningBufferUploader;

    // Present only when source has morph targets
    bool hasBlendshapes = false;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendshapeBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> BlendshapeBufferUploader;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW  IndexBufferView()  const;
    // Build a vertex buffer view pointing at an arbitrary resource with the same stride/size
    D3D12_VERTEX_BUFFER_VIEW MakeVertexBufferView(ID3D12Resource* vb) const;

    void DisposeUploaders();
};
