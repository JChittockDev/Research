#include "PhysicsPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../RenderItem.h"
#include "../Resources/MeshAnimationResource.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

PhysicsPass::PhysicsPass(const PhysicsPassResources& res) : mRes(res) {}

void PhysicsPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    auto& renderItems = ctx.renderItemLayers->at("Opaque");
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        if (renderItems[i]->AnimationInstance != nullptr && renderItems[i]->Simulation)
        {
            ctx.cmdList->SetComputeRootSignature(mRes.meshTransfer);
            ctx.cmdList->SetPipelineState(mRes.psoMeshTransfer);
            ComputeMeshTransfer(ctx.cmdList, renderItems[i], fr);

            ctx.cmdList->SetComputeRootSignature(mRes.tension);
            ctx.cmdList->SetPipelineState(mRes.psoTension);
            ComputeTension(ctx.cmdList, renderItems[i], fr);

            ComputePBD(ctx.cmdList, renderItems[i], fr);

            ctx.cmdList->SetComputeRootSignature(mRes.simMeshTransfer);
            ctx.cmdList->SetPipelineState(mRes.psoSimMeshTransfer);
            ComputeSimMeshTransfer(ctx.cmdList, renderItems[i], fr);

            ctx.cmdList->SetComputeRootSignature(mRes.triangleNormal);
            ctx.cmdList->SetPipelineState(mRes.psoTriangleNormal);
            ComputeTriangleNormals(ctx.cmdList, renderItems[i], fr);

            ctx.cmdList->SetComputeRootSignature(mRes.vertexNormal);
            ctx.cmdList->SetPipelineState(mRes.psoVertexNormal);
            ComputeVertexNormals(ctx.cmdList, renderItems[i], fr);
        }
    }
}

void PhysicsPass::ComputeMeshTransfer(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshTransferBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void PhysicsPass::ComputeTension(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshVertexNeighbourBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(VertexNeighbours));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshTensionBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshTensionBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector4));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshTensionBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputePBD(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootSignature(mRes.force);
    cmdList->SetPipelineState(mRes.psoForce);
    ComputeForce(cmdList, ri, currentFrameResource);

    cmdList->SetComputeRootSignature(mRes.preSolve);
    cmdList->SetPipelineState(mRes.psoPreSolve);
    ComputePreSolve(cmdList, ri, currentFrameResource);

    for (UINT i = 0; i < 1; ++i)
    {
        cmdList->SetComputeRootSignature(mRes.constraintSolve);
        cmdList->SetPipelineState(mRes.psoConstraintSolve);
        ComputeConstraintSolve(cmdList, ri, currentFrameResource);

        cmdList->SetComputeRootSignature(mRes.postSolve);
        cmdList->SetPipelineState(mRes.psoPostSolve);
        ComputePostSolve(cmdList, ri, currentFrameResource);
    }
}

void PhysicsPass::ComputeForce(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(1, ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputePreSolve(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputeConstraintSolve(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshConstraintIDsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshConstraintStart * sizeof(Edge));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshConstraintsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshConstraintStart * sizeof(float));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT3));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(4, ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshConstraintCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputePostSolve(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshVertexColorBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector4));
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->SimMeshTensionBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector4));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT3));
    cmdList->SetComputeRootShaderResourceView(3, ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));
    cmdList->SetComputeRootShaderResourceView(4, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(5, ri->Geo->SimMeshVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(6, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(7, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

    // Copy Data
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get());
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), ri->Geo->SimMeshNullSolverCountBufferGPU.Get());

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputeSimMeshTransfer(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->MeshTransferBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputeTriangleNormals(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->IndexBufferGPU->GetGPUVirtualAddress() + ri->IndexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->TriangleStart * sizeof(TangentNormals));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->TriangleCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsPass::ComputeVertexNormals(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->TriangleAdjacencyBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Neighbours));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->TriangleStart * sizeof(TangentNormals));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->VertexNormalBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}
