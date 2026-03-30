#include "BlendshapeDeformer.h"
#include "../Resources/FrameResource.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

BlendshapeDeformer::BlendshapeDeformer(
    ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    RenderMeshAsset* asset, UINT cbIndex,
    std::shared_ptr<BlendshapeController> controller,
    ID3D12RootSignature* rootSig, ID3D12PipelineState* pso)
    : mController(std::move(controller)), mCBIndex(cbIndex),
      mRootSig(rootSig), mPSO(pso)
{
    // BlendedVertexBufferGPU starts as a copy of the base-pose vertices
    // (SkinDeformer will CopyResource-reset this at end of each frame)
    Microsoft::WRL::ComPtr<ID3D12Resource> uploader;
    std::vector<uint8_t> zeroData(asset->VertexBufferByteSize, 0);
    BlendedVertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        device, cmdList, zeroData.data(), asset->VertexBufferByteSize, uploader);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        BlendedVertexBufferGPU.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void BlendshapeDeformer::Execute(ID3D12GraphicsCommandList* cmd, const DeformContext& ctx)
{
    auto& args = ctx.MeshAsset->DrawArgs.at(*ctx.SubsetName);

    UINT blendCBSize = d3dUtil::CalcConstantBufferByteSize(sizeof(BlendConstants));
    auto blendCB = ctx.Frame->BlendCB->Resource();

    cmd->SetComputeRootSignature(mRootSig);
    cmd->SetPipelineState(mPSO);

    for (UINT i = 0; i < args.BlendshapeCount; ++i) {
        cmd->SetComputeRoot32BitConstant(0, args.BlendshapeStart + i, 0);
        cmd->SetComputeRootConstantBufferView(1,
            blendCB->GetGPUVirtualAddress() + mCBIndex * blendCBSize);
        cmd->SetComputeRootShaderResourceView(2,
            ctx.MeshAsset->BlendshapeBufferGPU->GetGPUVirtualAddress()
            + (args.BlendshapeVertexStart + args.BlendshapeSubsets[i].VertexStart)
            * sizeof(BlendshapeVertex));

        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            BlendedVertexBufferGPU.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
        cmd->SetComputeRootUnorderedAccessView(3,
            BlendedVertexBufferGPU->GetGPUVirtualAddress()
            + args.VertexStart * sizeof(Vertex));

        cmd->Dispatch((args.BlendshapeSubsets[i].VertexCount + 31) / 32, 1, 1);

        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            BlendedVertexBufferGPU.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    }
}
