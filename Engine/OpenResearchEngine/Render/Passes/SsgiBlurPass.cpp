#include "SsgiBlurPass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/SsgiPassResource.h"

SsgiBlurPass::SsgiBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                            GBufferPassResource* gBuffer, SsgiPassResource* ssgi)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsgi(ssgi)
{}

void SsgiBlurPass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{
    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };


    mSsgi->SetResourceState(mSsgi->kGiResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsgiVerticalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetGpuSRVDescriptorHandle(mSsgi->kDepthResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGpuSRVDescriptorHandle(mSsgi->kGiResource));
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetCpuRTVDescriptorHandle(mSsgi->kGiVerticalBlurResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetCpuRTVDescriptorHandle(mSsgi->kGiHorizontalBlurResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    mSsgi->SetResourceState(mSsgi->kGiResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    mSsgi->SetResourceState(mSsgi->kGiVerticalBlurResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // Horizontal blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsgiHorizontalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetGpuSRVDescriptorHandle(mSsgi->kDepthResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGpuSRVDescriptorHandle(mSsgi->kGiVerticalBlurResource));
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetCpuRTVDescriptorHandle(mSsgi->kGiHorizontalBlurResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetCpuRTVDescriptorHandle(mSsgi->kGiHorizontalBlurResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    mSsgi->SetResourceState(mSsgi->kGiVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
