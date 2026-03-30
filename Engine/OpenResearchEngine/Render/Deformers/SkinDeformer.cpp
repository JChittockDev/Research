#include "SkinDeformer.h"
#include "../Resources/FrameResource.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

SkinDeformer::SkinDeformer(
    ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    RenderMeshAsset* asset, UINT cbIndex,
    std::shared_ptr<SkinningController> controller,
    ID3D12RootSignature* rootSig, ID3D12PipelineState* pso)
    : mController(std::move(controller)), mCBIndex(cbIndex),
      mRootSig(rootSig), mPSO(pso),
      mVertexByteStride(asset->VertexByteStride),
      mVertexBufferByteSize(asset->VertexBufferByteSize)
{
    // SkinnedVertexBufferGPU — same size as the full mesh vertex buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> uploader;
    std::vector<uint8_t> zeroData(asset->VertexBufferByteSize, 0);
    SkinnedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        device, cmdList, zeroData.data(), asset->VertexBufferByteSize, uploader);
    // Immediately transition to VERTEX_AND_CONSTANT_BUFFER (initial draw state)
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
    // uploader held alive until command list executes; caller must flush before releasing
}

void SkinDeformer::Execute(ID3D12GraphicsCommandList* cmd, const DeformContext& ctx)
{
    auto& args = ctx.MeshAsset->DrawArgs.at(*ctx.SubsetName);

    // Use blended vertices if BlendshapeDeformer ran; else use raw asset vertices
    ID3D12Resource* inputVB = ctx.BlendedVertexBuffer
        ? ctx.BlendedVertexBuffer
        : ctx.MeshAsset->VertexBufferGPU.Get();

    UINT skinnedCBSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = ctx.Frame->SkinnedCB->Resource();

    cmd->SetComputeRootSignature(mRootSig);
    cmd->SetPipelineState(mPSO);
    cmd->SetComputeRootConstantBufferView(0,
        skinnedCB->GetGPUVirtualAddress() + mCBIndex * skinnedCBSize);
    cmd->SetComputeRootShaderResourceView(1,
        inputVB->GetGPUVirtualAddress() + args.VertexStart * sizeof(Vertex));
    cmd->SetComputeRootShaderResourceView(2,
        ctx.MeshAsset->SkinningBufferGPU->GetGPUVirtualAddress()
        + args.VertexStart * sizeof(SkinningInfo));

    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmd->SetComputeRootUnorderedAccessView(3,
        SkinnedVertexBufferGPU->GetGPUVirtualAddress()
        + args.VertexStart * sizeof(Vertex));

    cmd->Dispatch((args.VertexCount + 63) / 64, 1, 1);

    // Reset BlendedVertexBuffer to base pose for next frame
    if (ctx.BlendedVertexBuffer) {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            ctx.MeshAsset->VertexBufferGPU.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_SOURCE));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            ctx.BlendedVertexBuffer,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_DEST));
        cmd->CopyResource(ctx.BlendedVertexBuffer,
            ctx.MeshAsset->VertexBufferGPU.Get());
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            ctx.MeshAsset->VertexBufferGPU.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            ctx.BlendedVertexBuffer,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    }

    cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        SkinnedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}
