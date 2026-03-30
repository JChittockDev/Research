#pragma once
#include "RenderMeshAsset.h"
#include "../Deformers/DeformationGraph.h"
#include <d3d12.h>

class MeshInstance {
public:
    MeshInstance(RenderMeshAsset* asset, DeformationGraph graph);

    RenderMeshAsset*    Asset() const { return mAsset; }
    DeformationGraph&   Graph()       { return mGraph; }
    const DeformationGraph& Graph()   const { return mGraph; }

    // Returns graph output if non-empty, otherwise asset VertexBufferGPU
    ID3D12Resource* FinalVertexBuffer() const;

private:
    RenderMeshAsset*  mAsset;  // non-owning
    DeformationGraph  mGraph;
};
