#include "GBufferPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/RenderTextures.h"
#include "../DrawRenderItems.h"

GBufferPass::GBufferPass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBufferPassResource*             gBuffer,
    RenderTextures*      renderTextures)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mRenderTextures(renderTextures)
{}

void GBufferPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kPositionResource),    clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kNormalResource),      clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kViewNormalResource),  clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kAlbedoSpecResource),  clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kReflectionResource),  clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kMaterialIdResource),  clearColor, 0, nullptr);
    ctx.cmdList->ClearRenderTargetView(mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kTangentResource),     clearColor, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kPositionResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kNormalResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kViewNormalResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kAlbedoSpecResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kReflectionResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kMaterialIdResource),
        mGBuffer->GetCpuRTVDescriptorHandle(mGBuffer->kTangentResource)
    };

    ctx.cmdList->ClearDepthStencilView(ctx.dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(7, rtvs, false, &ctx.dsv);

    auto passCBAddress   = fr->PassCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress     = fr->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    ctx.cmdList->SetGraphicsRootConstantBufferView(1, passCBAddress);
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, matBAddress);
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mRenderTextures->GetStartGpuSrv());
    ctx.cmdList->SetPipelineState(mPso);

    DrawRenderItems(ctx.cmdList, ctx.renderItemLayers->at("Opaque"), fr);
}
