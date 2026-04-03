#include "RadiancePass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBuffer.h"
#include "../Resources/ShadowResources.h"
#include "../Resources/RadianceResources.h"
#include "../../D3D12/D3DUtil.h"

RadiancePass::RadiancePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBuffer*             gBuffer,
    ShadowResources*     shadowResources,
    RadianceResources*   radianceResources)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer),
      mShadowResources(shadowResources), mRadianceResources(radianceResources)
{}

void RadiancePass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->ObjectCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, fr->MaterialBuffer->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetPositionGpuSrv());
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    UINT radianceCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(RadianceConstants));
    auto radianceCB = fr->RadianceCB->Resource();

    for (int i = 0; i < (int)ctx.lights->LightTransforms.size(); i++)
    {
        ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            mRadianceResources->radianceMaps[i]->GetDiffuseReflectance().Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
        ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            mRadianceResources->radianceMaps[i]->GetSpecularReflectance().Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

        ctx.cmdList->SetGraphicsRootConstantBufferView(1,
            radianceCB->GetGPUVirtualAddress() + i * radianceCBByteSize);
        ctx.cmdList->SetGraphicsRootDescriptorTable(4, mShadowResources->shadowMaps[i]->Srv());

        float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ctx.cmdList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetDiffuseReflectanceCpuRtv(), clearColor, 0, nullptr);
        ctx.cmdList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetSpecularReflectanceCpuRtv(), clearColor, 0, nullptr);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
            mRadianceResources->radianceMaps[i]->GetDiffuseReflectanceCpuRtv(),
            mRadianceResources->radianceMaps[i]->GetSpecularReflectanceCpuRtv()
        };
        ctx.cmdList->OMSetRenderTargets(2, rtvs, false, nullptr);
        ctx.cmdList->SetPipelineState(mPso);
        ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
        ctx.cmdList->IASetIndexBuffer(nullptr);
        ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx.cmdList->DrawInstanced(6, 1, 0, 0);

        ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            mRadianceResources->radianceMaps[i]->GetDiffuseReflectance().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        ctx.cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            mRadianceResources->radianceMaps[i]->GetSpecularReflectance().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
}
