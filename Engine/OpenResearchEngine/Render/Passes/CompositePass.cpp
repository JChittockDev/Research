#include "CompositePass.h"
#include "../RenderContext.h"
#include "../RenderPassConstantBuffers.h"
#include "../Resources/LightingPassResource.h"
#include "../Resources/SsgiPassResource.h"
#include "../../D3D12/D3Dx12.h"

CompositePass::CompositePass(
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    LightingPassResource*            lighting,
    SsgiPassResource*                ssgi)
    : mRootSig(rootSig), mPso(pso), mLighting(lighting), mSsgi(ssgi)
{}

void CompositePass::Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr)
{

	mSsgi->SetResourceState(mSsgi->kGiHorizontalBlurResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ctx.cmdList->SetGraphicsRootSignature(mRootSig);
	ctx.cmdList->RSSetViewports(1, &ctx.viewport);
	ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
	ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->ObjectCB->Resource()->GetGPUVirtualAddress());
	ctx.cmdList->SetGraphicsRootDescriptorTable(1, mLighting->GetGpuSRVDescriptorHandle(mLighting->kLightingResource));
	ctx.cmdList->SetGraphicsRootDescriptorTable(2, mSsgi->GetGpuSRVDescriptorHandle(mSsgi->kGiHorizontalBlurResource));

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	BarrierTransition::Transition(ctx.cmdList, ctx.backBuffer,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET,
        "BackBuffer", "CompositePass");
	ctx.cmdList->ClearRenderTargetView(ctx.backBufferRtv, clearValue, 0, nullptr);
	ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRtv, true, nullptr);
	ctx.cmdList->SetPipelineState(mPso);
	ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
	ctx.cmdList->IASetIndexBuffer(nullptr);
	ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx.cmdList->DrawInstanced(6, 1, 0, 0);

    mSsgi->SetResourceState(mSsgi->kGiHorizontalBlurResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
