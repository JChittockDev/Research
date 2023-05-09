#include "RenderCore.h"

void RenderCore::SsaoPass(int blurCount, FrameResource* currentFrameResource)
{
    commandList->RSSetViewports(1, &screenViewport);
    commandList->RSSetScissorRects(1, &screenScissorRect);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ssao->AmbientMap(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    commandList->ClearRenderTargetView(ssao->AmbientMapRtv(), clearValue, 0, nullptr);
    commandList->OMSetRenderTargets(1, &ssao->AmbientMapRtv(), true, nullptr);
    auto ssaoCBAddress = currentFrameResource->SsaoCB->Resource()->GetGPUVirtualAddress();
    commandList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);
    commandList->SetGraphicsRoot32BitConstant(1, 0, 0);
    commandList->SetGraphicsRootDescriptorTable(2, ssao->NormalMapSrv());
    commandList->SetGraphicsRootDescriptorTable(3, ssao->RandomVectorMapSrv());
    commandList->SetPipelineState(ssao->GetPSO());
    commandList->IASetVertexBuffers(0, 0, nullptr);
    commandList->IASetIndexBuffer(nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(6, 1, 0, 0);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ssao->AmbientMap(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
    ssao->BlurAmbientMap(commandList, currentFrameResource, blurCount);
}