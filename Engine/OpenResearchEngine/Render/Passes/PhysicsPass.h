#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>
#include <memory>

struct RenderItem;

struct PhysicsPassResources
{
    ID3D12RootSignature* meshTransfer    = nullptr;
    ID3D12RootSignature* simMeshTransfer = nullptr;
    ID3D12RootSignature* triangleNormal  = nullptr;
    ID3D12RootSignature* vertexNormal    = nullptr;
    ID3D12RootSignature* force           = nullptr;
    ID3D12RootSignature* preSolve        = nullptr;
    ID3D12RootSignature* postSolve       = nullptr;
    ID3D12RootSignature* constraintSolve = nullptr;
    ID3D12RootSignature* tension         = nullptr;

    ID3D12PipelineState* psoMeshTransfer    = nullptr;
    ID3D12PipelineState* psoSimMeshTransfer = nullptr;
    ID3D12PipelineState* psoTriangleNormal  = nullptr;
    ID3D12PipelineState* psoVertexNormal    = nullptr;
    ID3D12PipelineState* psoForce           = nullptr;
    ID3D12PipelineState* psoPreSolve        = nullptr;
    ID3D12PipelineState* psoPostSolve       = nullptr;
    ID3D12PipelineState* psoConstraintSolve = nullptr;
    ID3D12PipelineState* psoTension         = nullptr;
};

class PhysicsPass : public IRenderPass
{
public:
    explicit PhysicsPass(const PhysicsPassResources& res);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Physics"; }

private:
    void ComputeMeshTransfer   (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeTension        (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputePBD            (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeForce          (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputePreSolve       (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeConstraintSolve(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputePostSolve      (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeSimMeshTransfer(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeTriangleNormals(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);
    void ComputeVertexNormals  (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*);

    PhysicsPassResources mRes;
};
