#include "SsaoBlurPass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/GBuffer.h"
#include "../../Render/Resources/Ssao.h"

SsaoBlurPass::SsaoBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                            GBuffer* gBuffer, Ssao* ssao)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSsao(ssao)
{}

void SsaoBlurPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto ssaoVerticalBlurCBAddress = fr->SsaoVerticalBlurCB->Resource()->GetGPUVirtualAddress();
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, ssaoVerticalBlurCBAddress);

    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsao->GetDepthGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mSsao->GetAmbientGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSsao->GetAmbientVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSsao->GetAmbientVerticalBlurCpuRtv(), true, nullptr);

    ctx.cmdList->SetPipelineState(mPso);

    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    //////////////////////////////////////////////////////////////////////////////////////////////////

    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto ssaoHorizontalBlurCBAddress = fr->SsaoHorizontalBlurCB->Resource()->GetGPUVirtualAddress();
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, ssaoHorizontalBlurCBAddress);

    ctx.cmdList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
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
