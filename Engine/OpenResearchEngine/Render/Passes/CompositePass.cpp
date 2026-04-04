#include "CompositePass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/Lighting.h"
#include "../Resources/Ssgi.h"
#include "../../D3D12/D3Dx12.h"

CompositePass::CompositePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    LightingPassInfo*            lighting,
    Ssgi*                ssgi)
    : mRootSig(rootSig), mPso(pso), mLighting(lighting), mSsgi(ssgi)
{}

void CompositePass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->ObjectCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mLighting->GetGpuSRVDescriptorHandle(mLighting->kLightingResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetGIHorizontalBlurGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        ctx.backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    ctx.cmdList->ClearRenderTargetView(ctx.backBufferRtv, clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRtv, true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
