#include "LightingPass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/GBuffer.h"
#include "../../Render/Resources/SSS.h"
#include "../../Render/Resources/RadianceResources.h"
#include "../../Render/Resources/Lighting.h"
#include "../../D3D12/d3dx12.h"

LightingPass::LightingPass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBuffer*             gBuffer,
    SSS*                 sss,
    RadianceResources*   radianceResources,
    Lighting*            lighting)
    : mRootSig(rootSig), mPso(pso),
      mGBuffer(gBuffer), mSss(sss),
      mRadianceResources(radianceResources), mLighting(lighting)
{}

void LightingPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    auto matBuffer = fr->MaterialBuffer->Resource();
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    auto objectCBAddress = fr->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto passCBAddress   = fr->PassCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress     = fr->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSSBlur().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    ctx.cmdList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    ctx.cmdList->SetGraphicsRootConstantBufferView(1, passCBAddress);
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, matBAddress);
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetPositionGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(4, mSss->GetSSSBlurGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(5, mRadianceResources->GetStartGpuSrv());

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mLighting->GetLightingCpuRtv(), clearColor, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mLighting->GetLightingCpuRtv(), true, nullptr);

    ctx.cmdList->SetPipelineState(mPso);

    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSSBlur().Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}
