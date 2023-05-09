#include "RenderCore.h"

void RenderCore::DepthPass(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, FrameResource* currentFrameResource)
{
    commandList->RSSetViewports(1, &screenViewport);
    commandList->RSSetScissorRects(1, &screenScissorRect);

    auto normalMap = ssao->NormalMap();
    auto normalMapRtv = ssao->NormalMapRtv();

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    commandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    commandList->OMSetRenderTargets(1, &normalMapRtv, true, &dsv);
    auto passCB = currentFrameResource->PassCB->Resource();
    commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
    commandList->SetPipelineState(psos->at("drawNormals").Get());
    SetRenderItems(commandList, renderItemLayers->at("Opaque"), currentFrameResource);
    commandList->SetPipelineState(psos->at("skinnedDrawNormals").Get());
    SetRenderItems(commandList, renderItemLayers->at("SkinnedOpaque"), currentFrameResource);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}