#include "AnimationPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../RenderItem.h"
#include "../Resources/MeshAnimationResource.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

AnimationPass::AnimationPass(const AnimationPassResources& res) : mRes(res) {}

void AnimationPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    auto& renderItems = ctx.renderItemLayers->at("Opaque");
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        if (renderItems[i]->AnimationInstance != nullptr)
        {
            ctx.cmdList->SetComputeRootSignature(mRes.blend);
            ctx.cmdList->SetPipelineState(mRes.psoBlend);
            ComputeBlendshapes(ctx.cmdList, renderItems[i], fr);

            ctx.cmdList->SetComputeRootSignature(mRes.skinned);
            ctx.cmdList->SetPipelineState(mRes.psoSkinned);
            ComputeSkinning(ctx.cmdList, renderItems[i], fr);
        }
    }
}

void AnimationPass::ComputeBlendshapes(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT blendCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(BlendConstants));
    auto blendCB = currentFrameResource->BlendCB->Resource();

    for (UINT i = 0; i < ri->BlendshapeCount; ++i)
    {
        cmdList->SetComputeRoot32BitConstant(0, ri->BlendshapeStart + i, 0);
        cmdList->SetComputeRootConstantBufferView(1, blendCB->GetGPUVirtualAddress() + ri->BlendCBIndex * blendCBByteSize);
        cmdList->SetComputeRootShaderResourceView(2, ri->Geo->BlendshapeBufferGPU->GetGPUVirtualAddress() + (ri->BlendshapeVertexStart + ri->BlendshapeSubsets[i].VertexStart) * sizeof(BlendshapeVertex));

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
        cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

        const UINT threadGroupSizeX = 32;
        const UINT threadGroupSizeY = 1;
        const UINT threadGroupSizeZ = 1;
        cmdList->Dispatch((ri->BlendshapeSubsets[i].VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    }
}

void AnimationPass::ComputeSkinning(ID3D12GraphicsCommandList* cmdList, const std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();

    cmdList->SetComputeRootConstantBufferView(0, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->SkinningBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(SkinningInfo));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

    // Copy Data
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), ri->Geo->VertexBufferGPU.Get());

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}
