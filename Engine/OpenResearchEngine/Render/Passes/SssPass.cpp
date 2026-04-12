#include "SssPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBufferPassResource.h"
#include "../Resources/SssPassResource.h"
#include "../RenderPassResourceArray.h"

SssPass::SssPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                 GBufferPassResource* gBuffer, SssPassResource* sss, RenderPassResourceArray* radianceResources)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSss(sss), mRadianceResources(radianceResources)
{}

void SssPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SssCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootShaderResourceView(1, fr->MaterialBuffer->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kPositionResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kAlbedoSpecResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(4, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kTangentResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(5, mGBuffer->GetGpuSRVDescriptorHandle(mGBuffer->kMaterialIdResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(6, mSss->GetGpuSRVDescriptorHandle(mSss->kRandomVectorResource));
    ctx.cmdList->SetGraphicsRootDescriptorTable(7, mRadianceResources->GetGpuDescriptorHandleStart("SRV"));

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSss->GetCpuRTVDescriptorHandle(mSss->kSssResource), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSss->GetCpuRTVDescriptorHandle(mSss->kSssResource), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
