#include "SsgiPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/LightingPassResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/Ssgi.h"

SsgiPass::SsgiPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
    LightingPassResource* lighting, GBufferPassResource* gBuffer, Ssgi* ssgi)
    : mRootSig(rootSig), mPso(pso), mLighting(lighting), mGBuffer(gBuffer), mSsgi(ssgi)
{}

void SsgiPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SsgiCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mLighting->GetGpuSRVDescriptorHandle(mLighting->kLightingResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kNormalResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetRandomVectorGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetGICpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetGICpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
