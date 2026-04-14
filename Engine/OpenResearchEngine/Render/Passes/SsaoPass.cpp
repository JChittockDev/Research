#include "SsaoPass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/SsaoPassResource.h"

SsaoPass::SsaoPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                   GBufferPassResource* gBuffer, SsaoPassResource* ssao)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsao(ssao)
{}

void SsaoPass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsaoCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetGpuSRVDescriptorHandle(mSsao->kRandomVectorResource));

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetCpuRTVDescriptorHandle(mSsao->kAmbientResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
