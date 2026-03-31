#include "SssBlurPass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/SSS.h"
#include "../../D3D12/d3dx12.h"

SssBlurPass::SssBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, SSS* sss)
    : mRootSig(rootSig), mPso(pso), mSss(sss)
{}

void SssBlurPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSS().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    auto sssVerticalBlurCBAddress = fr->SssBlurCB->Resource()->GetGPUVirtualAddress();
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, sssVerticalBlurCBAddress);
    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mSss->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSss->GetSSSGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSss->GetSSSBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSss->GetSSSBlurCpuRtv(), true, nullptr);

    ctx.cmdList->SetPipelineState(mPso);

    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSS().Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}
