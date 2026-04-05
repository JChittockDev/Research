#include "SsgiBlurPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/Ssgi.h"

SsgiBlurPass::SsgiBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                            GBufferPassResource* gBuffer, Ssgi* ssgi)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsgi(ssgi)
{}

void SsgiBlurPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsgi->GetGI().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Vertical blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsgiVerticalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIGpuSrv());
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetGIVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetGIVerticalBlurCpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsgi->GetGIVerticalBlur().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsgi->GetGI().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Horizontal blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsgiHorizontalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIVerticalBlurGpuSrv());
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetGIHorizontalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetGIHorizontalBlurCpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        mSsgi->GetGIHorizontalBlur().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
