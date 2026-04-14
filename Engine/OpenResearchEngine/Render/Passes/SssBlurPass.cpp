#include "SssBlurPass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/SssPassResource.h"
#include "../../D3D12/D3Dx12.h"

SssBlurPass::SssBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, SssPassResource* sss)
    : mRootSig(rootSig), mPso(pso), mSss(sss)
{}

void SssBlurPass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSss->GetResource(mSss->kSssResource).Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SssBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mSss->GetGpuSRVDescriptorHandle(mSss->kDepthResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSss->GetGpuSRVDescriptorHandle(mSss->kSssResource));

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSss->GetCpuRTVDescriptorHandle(mSss->kSssBlurResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSss->GetCpuRTVDescriptorHandle(mSss->kSssBlurResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSss->GetResource(mSss->kSssResource).Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}
