#include "SssPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"
#include "../Resources/GBuffer.h"
#include "../Resources/SSS.h"
#include "../Resources/RadianceResources.h"

SssPass::SssPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                 GBuffer* gBuffer, SSS* sss, RadianceResources* radianceResources)
    : mRootSig(rootSig), mPso(pso), mGBuffer(gBuffer), mSss(sss), mRadianceResources(radianceResources)
{}

void SssPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    ctx.cmdList->SetGraphicsRootSignature(mRootSig);
    ctx.cmdList->RSSetViewports(1, &ctx.viewport);
    ctx.cmdList->RSSetScissorRects(1, &ctx.scissorRect);
    ctx.cmdList->SetGraphicsRootConstantBufferView(0, fr->SssCB->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootShaderResourceView(1, fr->MaterialBuffer->Resource()->GetGPUVirtualAddress());
    ctx.cmdList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetPositionGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetAlbedoSpecGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(4, mGBuffer->GetTangentGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(5, mGBuffer->GetMaterialIdGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(6, mSss->GetRandomVectorGpuSrv());
    ctx.cmdList->SetGraphicsRootDescriptorTable(7, mRadianceResources->GetStartGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(mSss->GetSSSCpuRtv(), clearValue, 0, nullptr);
    ctx.cmdList->OMSetRenderTargets(1, &mSss->GetSSSCpuRtv(), true, nullptr);
    ctx.cmdList->SetPipelineState(mPso);
    ctx.cmdList->IASetVertexBuffers(0, 0, nullptr);
    ctx.cmdList->IASetIndexBuffer(nullptr);
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(6, 1, 0, 0);
}
