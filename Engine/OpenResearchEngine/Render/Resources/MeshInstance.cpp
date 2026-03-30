#include "MeshInstance.h"

MeshInstance::MeshInstance(RenderMeshAsset* asset, DeformationGraph graph)
    : mAsset(asset), mGraph(std::move(graph)) {}

ID3D12Resource* MeshInstance::FinalVertexBuffer() const
{
    ID3D12Resource* graphOut = mGraph.FinalVertexBuffer();
    return graphOut ? graphOut : mAsset->VertexBufferGPU.Get();
}
