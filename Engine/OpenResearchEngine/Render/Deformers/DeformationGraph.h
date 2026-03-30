#pragma once
#include "IDeformer.h"
#include <vector>
#include <memory>
#include <d3d12.h>

class DeformationGraph {
public:
    void AddDeformer(std::unique_ptr<IDeformer> d);
    void Execute(ID3D12GraphicsCommandList* cmd, const DeformContext& baseCtx);

    // Returns the GPU buffer to use as vertex buffer for rendering:
    //   PhysicsDeformer present -> VertexNormalBufferGPU  (VERTEX_AND_CONSTANT_BUFFER)
    //   SkinDeformer only       -> SkinnedVertexBufferGPU (VERTEX_AND_CONSTANT_BUFFER)
    //   Empty graph (static)    -> nullptr (caller falls back to asset VertexBufferGPU)
    ID3D12Resource* FinalVertexBuffer() const;

    bool IsEmpty() const { return mDeformers.empty(); }

private:
    std::vector<std::unique_ptr<IDeformer>> mDeformers;
};
