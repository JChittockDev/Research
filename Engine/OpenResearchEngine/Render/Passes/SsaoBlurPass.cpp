#include "SsaoBlurPass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/SsaoPassResource.h"

SsaoBlurPass::SsaoBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, GBufferPassResource* gBuffer, SsaoPassResource* ssao)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsao(ssao)
{}

void SsaoBlurPass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{
    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    mSsao->SetResourceState(mSsao->kAmbientResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    mSsao->SetResourceState(mSsao->kAmbientVerticalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Vertical blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsaoVerticalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetGpuSRVDescriptorHandle(mSsao->kDepthResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsao->GetGpuSRVDescriptorHandle(mSsao->kAmbientResource));
    ctx.cmdList->ClearRenderTargetView(mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientVerticalBlurResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientVerticalBlurResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

	mSsao->SetResourceState(mSsao->kAmbientResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mSsao->SetResourceState(mSsao->kAmbientVerticalBlurResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    mSsao->SetResourceState(mSsao->kAmbientHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Horizontal blur
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsaoHorizontalBlurCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetGpuSRVDescriptorHandle(mSsao->kDepthResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsao->GetGpuSRVDescriptorHandle(mSsao->kAmbientVerticalBlurResource));
    ctx.cmdList->ClearRenderTargetView(mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientHorizontalBlurResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientHorizontalBlurResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    mSsao->SetResourceState(mSsao->kAmbientHorizontalBlurResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
