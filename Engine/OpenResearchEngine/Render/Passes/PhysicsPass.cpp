#include "PhysicsPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"

PhysicsPass::PhysicsPass(const PhysicsPassResources& res) : mRes(res) {}

void PhysicsPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    // PBD simulation dispatch — pending update to DeformationGraph API
}

void PhysicsPass::ComputeMeshTransfer   (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeTension        (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputePBD            (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeForce          (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputePreSolve       (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeConstraintSolve(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputePostSolve      (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeSimMeshTransfer(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeTriangleNormals(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void PhysicsPass::ComputeVertexNormals  (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
