#include "RadiancePass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/ShadowResources.h"
#include "../Resources/RadianceResources.h"
#include "../../D3D12/D3DUtil.h"

RadiancePass::RadiancePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBufferPassResource*             gBuffer,
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
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kPositionResource));
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

    UINT radianceCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(RadianceConstants));
    auto radianceCB = fr->RadianceCB->Resource();

    for (int i = 0; i < (int)ctx.lights->LightTransforms.size(); i++)
    {
        mRadianceResources->radianceMaps[i]->SetResourceState(mRadianceResources->radianceMaps[i]->kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mRadianceResources->radianceMaps[i]->SetResourceState(mRadianceResources->radianceMaps[i]->kSpecularReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

        ctx.cmdList->SetGraphicsRootConstantBufferView(1,
            radianceCB->GetGPUVirtualAddress() + i * radianceCBByteSize);
        ctx.cmdList->SetGraphicsRootDescriptorTable(4, mShadowResources->shadowMaps[i]->Srv());

        float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ctx.cmdList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetCpuRTVDescriptorHandle(mRadianceResources->radianceMaps[i]->kDiffuseReflectanceResource), clearColor, 0, nullptr);
        ctx.cmdList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetCpuRTVDescriptorHandle(mRadianceResources->radianceMaps[i]->kSpecularReflectanceResource), clearColor, 0, nullptr);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
            mRadianceResources->radianceMaps[i]->GetCpuRTVDescriptorHandle(mRadianceResources->radianceMaps[i]->kDiffuseReflectanceResource),
            mRadianceResources->radianceMaps[i]->GetCpuRTVDescriptorHandle(mRadianceResources->radianceMaps[i]->kSpecularReflectanceResource)
        };
        ctx.cmdList->OMSetRenderTargets(2, rtvs, false, nullptr);
        ctx.cmdList->SetPipelineState(mPso);
        ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
        ctx.cmdList->IASetIndexBuffer(nullptr);
        ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx.cmdList->DrawInstanced(6, 1, 0, 0);

		mRadianceResources->radianceMaps[i]->SetResourceState(mRadianceResources->radianceMaps[i]->kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		mRadianceResources->radianceMaps[i]->SetResourceState(mRadianceResources->radianceMaps[i]->kSpecularReflectanceResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}
