#include "PhysicsDeformer.h"
#include "../Resources/RenderMeshAsset.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

PhysicsDeformer::PhysicsDeformer(
    ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    SimMeshAsset* simAsset, const PhysicsDeformerResources& res)
    : mSimAsset(simAsset), mRes(res), mVertexByteStride(0), mVertexBufferByteSize(0)
{
    UINT totalSimVerts = 0, totalVerts = 0, totalTris = 0;
    for (auto& [name, ss] : simAsset->SubsetData) {
        UINT sv = ss.SimMeshVertexStart + ss.SimMeshVertexCount;
        UINT rv = ss.VertexStart        + ss.VertexCount;
        UINT rt = ss.TriangleStart      + ss.TriangleCount;
        if (sv > totalSimVerts) totalSimVerts = sv;
        if (rv > totalVerts)    totalVerts    = rv;
        if (rt > totalTris)     totalTris     = rt;
    }

    auto makeNPSR = [&](UINT byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& out) {
        Microsoft::WRL::ComPtr<ID3D12Resource> up;
        std::vector<uint8_t> z(byteSize, 0);
        out = d3dUtil::CreateDefaultBuffer(device, cmdList, z.data(), byteSize, up);
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            out.Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    };

    makeNPSR(totalSimVerts * sizeof(Vertex),   SimMeshSkinnedVertexBufferGPU);
    makeNPSR(totalSimVerts * sizeof(Vertex),   SimMeshPreviousSkinnedVertexBufferGPU);
    makeNPSR(totalSimVerts * sizeof(Vector3),  SimMeshForceBufferGPU);
    makeNPSR(totalSimVerts * sizeof(Vector4),  SimMeshTensionBufferGPU);
    makeNPSR(totalSimVerts * sizeof(UINT3),    SimMeshSolverAccumulationBufferGPU);
    makeNPSR(totalSimVerts * sizeof(UINT),     SimMeshSolverCountBufferGPU);
    makeNPSR(totalSimVerts * sizeof(Vertex),   SimMeshSolverVertexBufferGPU);
    makeNPSR(totalSimVerts * sizeof(Vertex),   SimMeshConstraintsVertexBufferGPU);
    makeNPSR(totalVerts    * sizeof(Vertex),   TransformedVertexBufferGPU);
    makeNPSR(totalTris     * sizeof(TangentNormals), TriangleNormalBufferGPU);

    {
        Microsoft::WRL::ComPtr<ID3D12Resource> up;
        UINT sz = totalVerts * sizeof(Vertex);
        std::vector<uint8_t> z(sz, 0);
        VertexNormalBufferGPU = d3dUtil::CreateDefaultBuffer(device, cmdList, z.data(), sz, up);
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            VertexNormalBufferGPU.Get(),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
    }
}

void PhysicsDeformer::Execute(ID3D12GraphicsCommandList* cmd, const DeformContext& ctx)
{
    const SimSubset& ss = mSimAsset->SubsetData.at(*ctx.SubsetName);

    cmd->SetComputeRootSignature(mRes.meshTransfer);
    cmd->SetPipelineState(mRes.psoMeshTransfer);
    DispatchMeshTransfer(cmd, ctx, ss);

    cmd->SetComputeRootSignature(mRes.tension);
    cmd->SetPipelineState(mRes.psoTension);
    DispatchTension(cmd, ss);

    cmd->SetComputeRootSignature(mRes.force);
    cmd->SetPipelineState(mRes.psoForce);
    DispatchForce(cmd, ss);

    cmd->SetComputeRootSignature(mRes.preSolve);
    cmd->SetPipelineState(mRes.psoPreSolve);
    DispatchPreSolve(cmd, ss);

    cmd->SetComputeRootSignature(mRes.constraintSolve);
    cmd->SetPipelineState(mRes.psoConstraintSolve);
    DispatchConstraintSolve(cmd, ss);

    cmd->SetComputeRootSignature(mRes.postSolve);
    cmd->SetPipelineState(mRes.psoPostSolve);
    DispatchPostSolve(cmd, ss);

    cmd->SetComputeRootSignature(mRes.simMeshTransfer);
    cmd->SetPipelineState(mRes.psoSimMeshTransfer);
    DispatchSimMeshTransfer(cmd, ctx, ss);

    cmd->SetComputeRootSignature(mRes.triangleNormal);
    cmd->SetPipelineState(mRes.psoTriangleNormal);
    DispatchTriangleNormals(cmd, ctx, ss);

    cmd->SetComputeRootSignature(mRes.vertexNormal);
    cmd->SetPipelineState(mRes.psoVertexNormal);
    DispatchVertexNormals(cmd, ctx, ss);
}

void PhysicsDeformer::DispatchMeshTransfer(
    ID3D12GraphicsCommandList* cmd, const DeformContext& ctx, const SimSubset& ss)
{
    ID3D12Resource* skinnedVB = ctx.SkinnedVertexBuffer;
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(skinnedVB,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->SetComputeRootShaderResourceView(0,
        skinnedVB->GetGPUVirtualAddress() + ss.VertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(1,
        mSimAsset->SimMeshTransferBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(UINT));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->Dispatch((ss.SimMeshVertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(skinnedVB,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void PhysicsDeformer::DispatchTension(ID3D12GraphicsCommandList* cmd, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(1,
        mSimAsset->SimMeshVertexNeighbourBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(VertexNeighbours));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshTensionBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        SimMeshTensionBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vector4));
    cmd->Dispatch((ss.SimMeshVertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshTensionBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchForce(ID3D12GraphicsCommandList* cmd, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshPreviousSkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(1,
        SimMeshPreviousSkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshForceBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        SimMeshForceBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vector3));
    cmd->Dispatch((ss.SimMeshVertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshPreviousSkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshForceBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchPreSolve(ID3D12GraphicsCommandList* cmd, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        SimMeshForceBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vector3));
    cmd->SetComputeRootShaderResourceView(1,
        SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshConstraintsVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->Dispatch((ss.SimMeshVertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshConstraintsVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchConstraintSolve(
    ID3D12GraphicsCommandList* cmd, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        mSimAsset->SimMeshConstraintIDsBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshConstraintStart * sizeof(Edge));
    cmd->SetComputeRootShaderResourceView(1,
        mSimAsset->SimMeshConstraintsBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshConstraintStart * sizeof(float));
    cmd->SetComputeRootShaderResourceView(2,
        SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(3,
        SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(UINT3));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(4,
        SimMeshSolverCountBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(UINT));
    cmd->Dispatch((ss.SimMeshConstraintCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchPostSolve(ID3D12GraphicsCommandList* cmd, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        mSimAsset->SimMeshVertexColorBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vector4));
    cmd->SetComputeRootShaderResourceView(1,
        SimMeshTensionBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vector4));
    cmd->SetComputeRootShaderResourceView(2,
        SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(UINT3));
    cmd->SetComputeRootShaderResourceView(3,
        SimMeshSolverCountBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(UINT));
    cmd->SetComputeRootShaderResourceView(4,
        SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(5,
        mSimAsset->SimMeshVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(6,
        SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(7,
        SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->Dispatch((ss.SimMeshVertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    // Reset accumulation buffers from null (zero) source on SimMeshAsset
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSimAsset->SimMeshNullSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSimAsset->SimMeshNullSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
    cmd->CopyResource(SimMeshSolverAccumulationBufferGPU.Get(),
        mSimAsset->SimMeshNullSolverAccumulationBufferGPU.Get());
    cmd->CopyResource(SimMeshSolverCountBufferGPU.Get(),
        mSimAsset->SimMeshNullSolverCountBufferGPU.Get());
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSimAsset->SimMeshNullSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverAccumulationBufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSimAsset->SimMeshNullSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SimMeshSolverCountBufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchSimMeshTransfer(
    ID3D12GraphicsCommandList* cmd, const DeformContext& ctx, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshVertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(1,
        mSimAsset->MeshTransferBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(UINT));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        TransformedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        TransformedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(Vertex));
    cmd->Dispatch((ss.VertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        TransformedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchTriangleNormals(
    ID3D12GraphicsCommandList* cmd, const DeformContext& ctx, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        TransformedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(1,
        ctx.MeshAsset->IndexBufferGPU->GetGPUVirtualAddress()
        + ss.SimMeshIndexStart * sizeof(UINT));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        TriangleNormalBufferGPU.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(2,
        TriangleNormalBufferGPU->GetGPUVirtualAddress()
        + ss.TriangleStart * sizeof(TangentNormals));
    cmd->Dispatch((ss.TriangleCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        TriangleNormalBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void PhysicsDeformer::DispatchVertexNormals(
    ID3D12GraphicsCommandList* cmd, const DeformContext& ctx, const SimSubset& ss)
{
    cmd->SetComputeRootShaderResourceView(0,
        TransformedVertexBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(1,
        mSimAsset->TriangleAdjacencyBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(Neighbours));
    cmd->SetComputeRootShaderResourceView(2,
        TriangleNormalBufferGPU->GetGPUVirtualAddress()
        + ss.TriangleStart * sizeof(TangentNormals));
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        VertexNormalBufferGPU.Get(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(3,
        VertexNormalBufferGPU->GetGPUVirtualAddress()
        + ss.VertexStart * sizeof(Vertex));
    cmd->Dispatch((ss.VertexCount + 63) / 64, 1, 1);
    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        VertexNormalBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}
