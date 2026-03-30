#pragma once
#include <d3d12.h>
#include <string>

class RenderMeshAsset;
struct FrameResource;

enum class DeformerType { Blendshape, Skin, Physics };

// Passed to each IDeformer::Execute. Filled progressively by DeformationGraph
// between deformer calls so each deformer can read the previous stage's output.
struct DeformContext {
    RenderMeshAsset* MeshAsset  = nullptr;
    FrameResource*   Frame      = nullptr;
    const std::string* SubsetName = nullptr;  // key into MeshAsset->DrawArgs

    // Set by DeformationGraph before Execute loop if BlendshapeDeformer present:
    ID3D12Resource* BlendedVertexBuffer = nullptr;
    // Set by DeformationGraph after SkinDeformer runs (before PhysicsDeformer):
    ID3D12Resource* SkinnedVertexBuffer = nullptr;
};

class IDeformer {
public:
    virtual ~IDeformer() = default;
    virtual void Execute(ID3D12GraphicsCommandList*, const DeformContext&) = 0;
    virtual DeformerType Type() const = 0;
};
