#include "RenderCore.h"

void RenderCore::DiffusePass(const std::shared_ptr<std::unordered_map<std::string, std::pair<INT, UINT>>>& layoutIndexMap, FrameResource* currentFrameResource)
{
    commandList->RSSetViewports(1, &screenViewport);
    commandList->RSSetScissorRects(1, &screenScissorRect);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(cBB, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    commandList->ClearRenderTargetView(cbbv, DirectX::Colors::LightSteelBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    commandList->OMSetRenderTargets(1, &cbbv, true, &dsv);
    commandList->SetGraphicsRootDescriptorTable(5, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    auto passCB = currentFrameResource->PassCB->Resource();
    commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
    CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    skyTexDescriptor.Offset(layoutIndexMap->at("mSkyTexHeapIndex").first, layoutIndexMap->at("mSkyTexHeapIndex").second);
    commandList->SetGraphicsRootDescriptorTable(4, skyTexDescriptor);
    commandList->SetPipelineState(psos->at("opaque").Get());
    SetRenderItems(commandList, renderItemLayers->at("Opaque"), currentFrameResource);
    commandList->SetPipelineState(psos->at("skinnedOpaque").Get());
    SetRenderItems(commandList, renderItemLayers->at("SkinnedOpaque"), currentFrameResource);
    commandList->SetPipelineState(psos->at("sky").Get());
    SetRenderItems(commandList, renderItemLayers->at("Sky"), currentFrameResource);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(cBB, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}