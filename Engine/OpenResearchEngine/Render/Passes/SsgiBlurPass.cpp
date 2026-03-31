#include "SsgiBlurPass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/GBuffer.h"
#include "../../Render/Resources/Ssgi.h"

SsgiBlurPass::SsgiBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                            GBuffer* gBuffer, Ssgi* ssgi)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsgi(ssgi)
{}

void SsgiBlurPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto ssgiVerticalBlurCBAddress = fr->SsgiVerticalBlurCB->Resource()->GetGPUVirtualAddress();
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, ssgiVerticalBlurCBAddress);

    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSsgi->GetGIVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetGIVerticalBlurCpuRtv(), true, nullptr);

    ctx.cmdList->SetPipelineState(mPso);

    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto ssgiHorizontalBlurCBAddress = fr->SsgiHorizontalBlurCB->Resource()->GetGPUVirtualAddress();
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, ssgiHorizontalBlurCBAddress);

    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIVerticalBlurGpuSrv());

    ctx.cmdList->ClearRenderTargetView(mSsgi->GetGIHorizontalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsgi->GetGIHorizontalBlurCpuRtv(), true, nullptr);

    ctx.cmdList->SetPipelineState(mPso);

    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
