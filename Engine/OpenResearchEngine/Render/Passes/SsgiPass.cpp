#include "SsgiPass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/Lighting.h"
#include "../../Render/Resources/GBuffer.h"
#include "../../Render/Resources/Ssgi.h"

SsgiPass::SsgiPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                   Lighting* lighting, GBuffer* gBuffer, Ssgi* ssgi)
    : mRootSig(rootSig), mPso(pso), mLighting(lighting), mGBuffer(gBuffer), mSsgi(ssgi)
{}

void SsgiPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto ssgiCBAddress = fr->SsgiCB->Resource()->GetGPUVirtualAddress();

    ctx.cmdList->SetGraphicsRootConstantBufferView(0, ssgiCBAddress);
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mLighting->GetLightingGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetNormalGpuSrv());
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
