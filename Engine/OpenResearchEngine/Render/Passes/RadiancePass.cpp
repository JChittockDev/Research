#include "RadiancePass.h"
#include "../RenderContext.h"
#include "../../Render/Resources/FrameResource.h"
#include "../../Render/Resources/GBuffer.h"
#include "../../Render/Resources/ShadowResources.h"
#include "../../Render/Resources/RadianceResources.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/d3dx12.h"
#include "../../Common/Structures.h"

RadiancePass::RadiancePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBuffer*             gBuffer,
    ShadowResources*     shadowResources,
    RadianceResources*   radianceResources)
    : mRootSig(rootSig), mPso(pso),
      mGBuffer(gBuffer), mShadowResources(shadowResources),
      mRadianceResources(radianceResources)
{}

void RadiancePass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);

    auto objectCBAddress  = fr->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress      = fr->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    ctx.cmdList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, matBAddress);
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetPositionGpuSrv());

    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    const auto& lightTransforms = ctx.lights->LightTransforms;
    for (int i = 0; i < (int)lightTransforms.size(); i++)
    {
        UINT radianceCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(RadianceConstants));
        auto radianceCB = fr->RadianceCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS radianceCBAddress = radianceCB->GetGPUVirtualAddress() + (i) * radianceCBByteSize;
        ctx.cmdList->SetGraphicsRootConstantBufferView(1, radianceCBAddress);

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
    }
}
