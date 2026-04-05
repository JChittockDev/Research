#include "SsaoBlurPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/Ssao.h"

SsaoBlurPass::SsaoBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                            GBufferPassResource* gBuffer, Ssao* ssao)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsao(ssao)
{}

void SsaoBlurPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsao->GetAmbient().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Vertical blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsaoVerticalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsao->GetAmbientGpuSrv());
    ctx.cmdList->ClearRenderTargetView(mSsao->GetAmbientVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetAmbientVerticalBlurCpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsao->GetAmbientVerticalBlur().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsao->GetAmbient().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Horizontal blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsaoHorizontalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsao->GetAmbientVerticalBlurGpuSrv());
    ctx.cmdList->ClearRenderTargetView(mSsao->GetAmbientHorizontalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetAmbientHorizontalBlurCpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
