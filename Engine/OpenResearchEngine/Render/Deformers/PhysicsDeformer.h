#pragma once
#include "IDeformer.h"
#include "../Resources/SimMeshAsset.h"
#include "../../D3D12/D3DUtil.h"
#include <wrl/client.h>
#include <d3d12.h>

struct PhysicsDeformerResources {
    ID3D12RootSignature* meshTransfer;     ID3D12PipelineState* psoMeshTransfer;
    ID3D12RootSignature* tension;          ID3D12PipelineState* psoTension;
    ID3D12RootSignature* force;            ID3D12PipelineState* psoForce;
    ID3D12RootSignature* preSolve;         ID3D12PipelineState* psoPreSolve;
    ID3D12RootSignature* constraintSolve;  ID3D12PipelineState* psoConstraintSolve;
    ID3D12RootSignature* postSolve;        ID3D12PipelineState* psoPostSolve;
    ID3D12RootSignature* simMeshTransfer;  ID3D12PipelineState* psoSimMeshTransfer;
    ID3D12RootSignature* triangleNormal;   ID3D12PipelineState* psoTriangleNormal;
    ID3D12RootSignature* vertexNormal;     ID3D12PipelineState* psoVertexNormal;
};

class PhysicsDeformer : public IDeformer {
public:
    PhysicsDeformer(
        ID3D12Device*              device,
        ID3D12GraphicsCommandList* cmdList,
        SimMeshAsset*              simAsset,  // non-owning
        const PhysicsDeformerResources& res
    );

    void Execute(ID3D12GraphicsCommandList*, const DeformContext&) override;
    DeformerType Type() const override { return DeformerType::Physics; }

    SimMeshAsset* GetSimAsset() const { return mSimAsset; }

    // Per-instance solver buffers (all NON_PIXEL_SHADER_RESOURCE at rest)
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSkinnedVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshPreviousSkinnedVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshForceBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshTensionBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverAccumulationBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverCountBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshSolverVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> SimMeshConstraintsVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> TransformedVertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> TriangleNormalBufferGPU;
    // Final output — VERTEX_AND_CONSTANT_BUFFER after Execute()
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexNormalBufferGPU;

private:
    SimMeshAsset*           mSimAsset;
    PhysicsDeformerResources mRes;
    UINT mVertexByteStride;
    UINT mVertexBufferByteSize;

    void DispatchMeshTransfer   (ID3D12GraphicsCommandList*, const DeformContext&, const SimSubset&);
    void DispatchTension        (ID3D12GraphicsCommandList*, const SimSubset&);
    void DispatchForce          (ID3D12GraphicsCommandList*, const SimSubset&);
    void DispatchPreSolve       (ID3D12GraphicsCommandList*, const SimSubset&);
    void DispatchConstraintSolve(ID3D12GraphicsCommandList*, const SimSubset&);
    void DispatchPostSolve      (ID3D12GraphicsCommandList*, const SimSubset&);
    void DispatchSimMeshTransfer(ID3D12GraphicsCommandList*, const DeformContext&, const SimSubset&);
    void DispatchTriangleNormals(ID3D12GraphicsCommandList*, const DeformContext&, const SimSubset&);
    void DispatchVertexNormals  (ID3D12GraphicsCommandList*, const DeformContext&, const SimSubset&);
};
