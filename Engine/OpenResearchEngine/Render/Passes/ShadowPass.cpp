#include "ShadowPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/ShadowResources.h"
#include "../Resources/RenderTextures.h"
#include "../DrawRenderItems.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"

ShadowPass::ShadowPass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    ShadowResources*     shadowResources,
    RenderTextures*      renderTextures)
    : mRootSig(rootSig), mPso(pso), mShadowResources(shadowResources), mRenderTextures(renderTextures)
{}

void ShadowPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    auto matBuffer = fr->MaterialBuffer->Resource();
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mRenderTextures->GetStartGpuSrv());

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
    auto passCB = fr->PassCB->Resource();

    for (int i = 0; i < (int)ctx.lights->LightTransforms.size(); i++)
    {
        ctx.cmdList->RSSetViewports(1, &mShadowResources->shadowMaps[i]->Viewport());
        ctx.cmdList->RSSetScissorRects(1, &mShadowResources->shadowMaps[i]->ScissorRect());
        BarrierTransition::Transition(ctx.cmdList, mShadowResources->shadowMaps[i]->Resource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE,
            "ShadowMap", "ShadowPass");
        ctx.cmdList->ClearDepthStencilView(mShadowResources->shadowMaps[i]->Dsv(),
            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        ctx.cmdList->OMSetRenderTargets(0, nullptr, false, &mShadowResources->shadowMaps[i]->Dsv());
        ctx.cmdList->SetGraphicsRootConstantBufferView(1,
            passCB->GetGPUVirtualAddress() + (i + 1) * passCBByteSize);
        ctx.cmdList->SetPipelineState(mPso);
        DrawRenderItems(ctx.cmdList, ctx.renderItemLayers->at("Opaque"), fr);
        BarrierTransition::Transition(ctx.cmdList, mShadowResources->shadowMaps[i]->Resource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ,
            "ShadowMap", "ShadowPass");
    }
}
