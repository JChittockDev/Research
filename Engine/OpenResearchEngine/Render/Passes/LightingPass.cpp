#include "LightingPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/SssPassResource.h"
#include "../Resources/RadianceResources.h"
#include "../Resources/LightingPassResource.h"
#include "../../D3D12/D3Dx12.h"

LightingPass::LightingPass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    GBufferPassResource*             gBuffer,
    SssPassResource*                 sss,
    RadianceResources*   radianceResources,
    LightingPassResource*            lighting)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer),
      mSss(sss), mRadianceResources(radianceResources), mLighting(lighting)
{}

void LightingPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);

	mSss->SetResourceState(mSss->kSssBlurResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    mLighting->SetResourceState(mLighting->kLightingResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->ObjectCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootConstantBufferView(1, fr->PassCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootShaderResourceView(2, fr->MaterialBuffer->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kPositionResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(4, mSss->GetGpuSRVDescriptorHandle(mSss->kSssBlurResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(5, mRadianceResources->GetStartGpuSrv());

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mLighting->GetCpuRTVDescriptorHandle(mLighting->kLightingResource), clearColor, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mLighting->GetCpuRTVDescriptorHandle(mLighting->kLightingResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    mLighting->SetResourceState(mLighting->kLightingResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	mSss->SetResourceState(mSss->kSssBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
