#include "ShadowPass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/RenderTextures.h"
#include "../DrawRenderItems.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/D3Dx12.h"
#include "../../Common/Structures.h"
#include "../RenderPassResourceArray.h"
#include "../Resources/ShadowPassResource.h"

ShadowPass::ShadowPass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    RenderPassResourceArray*     shadowResources,
    RenderTextures*      renderTextures)
    : mRootSig(rootSig), mPso(pso), mShadowResources(shadowResources), mRenderTextures(renderTextures)
{}

void ShadowPass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{
    auto matBuffer = fr->MaterialBuffer->Resource();
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mRenderTextures->GetStartGpuSrv());

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
    auto passCB = fr->PassCB->Resource();

    for (int i = 0; i < (int)ctx.lights->LightTransforms.size(); i++)
    {
        ShadowPassResource* shadowResource = mShadowResources->Get<ShadowPassResource>(i);

        ctx.cmdList->RSSetViewports(1, &shadowResource->GetViewport());
        ctx.cmdList->RSSetScissorRects(1, &shadowResource->GetScissorRect());
        
        shadowResource->SetResourceState(shadowResource->kShadowResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        ctx.cmdList->ClearDepthStencilView(shadowResource->GetCpuDSVDescriptorHandle(shadowResource->kShadowResource), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        
        ctx.cmdList->OMSetRenderTargets(0, nullptr, false, &shadowResource->GetCpuDSVDescriptorHandle(shadowResource->kShadowResource));
        ctx.cmdList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress() + (i + 1) * passCBByteSize);
        ctx.cmdList->SetPipelineState(mPso);
        
        DrawRenderItems(ctx.cmdList, ctx.renderItemLayers->at("Opaque"), fr);

        shadowResource->SetResourceState(shadowResource->kShadowResource, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}
