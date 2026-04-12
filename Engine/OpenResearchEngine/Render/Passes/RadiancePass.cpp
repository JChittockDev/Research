#include "RadiancePass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/RadiancePassResource.h"
#include "../Resources/ShadowPassResource.h"
#include "../RenderPassResourceArray.h"
#include "../../D3D12/D3DUtil.h"

RadiancePass::RadiancePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBufferPassResource*             gBuffer,
    RenderPassResourceArray*     shadowResources,
    RenderPassResourceArray*   radianceResources)
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

		RadiancePassResource* radianceResource = mRadianceResources->Get<RadiancePassResource>(i);
		radianceResource->SetResourceState(radianceResource->kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		radianceResource->SetResourceState(radianceResource->kSpecularReflectanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		ctx.cmdList->SetGraphicsRootConstantBufferView(1,radianceCB->GetGPUVirtualAddress() + i * radianceCBByteSize);

		ShadowPassResource* shadowResource = mShadowResources->Get<ShadowPassResource>(i);
		ctx.cmdList->SetGraphicsRootDescriptorTable(4, shadowResource->GetGpuSRVDescriptorHandle(shadowResource->kShadowResource));

		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		ctx.cmdList->ClearRenderTargetView(radianceResource->GetCpuRTVDescriptorHandle(radianceResource->kDiffuseReflectanceResource), clearColor, 0, nullptr);
		ctx.cmdList->ClearRenderTargetView(radianceResource->GetCpuRTVDescriptorHandle(radianceResource->kSpecularReflectanceResource), clearColor, 0, nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
			radianceResource->GetCpuRTVDescriptorHandle(radianceResource->kDiffuseReflectanceResource),
			radianceResource->GetCpuRTVDescriptorHandle(radianceResource->kSpecularReflectanceResource)
		};
		ctx.cmdList->OMSetRenderTargets(2, rtvs, false, nullptr);
		ctx.cmdList->SetPipelineState(mPso);
		ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
		ctx.cmdList->IASetIndexBuffer(nullptr);
		ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx.cmdList->DrawInstanced(6, 1, 0, 0);

		radianceResource->SetResourceState(radianceResource->kDiffuseReflectanceResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		radianceResource->SetResourceState(radianceResource->kSpecularReflectanceResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}
