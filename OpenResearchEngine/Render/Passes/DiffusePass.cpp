#include "../../EngineApp.h"

void EngineApp::DiffusePass(const std::unordered_map<std::string, std::pair<INT, UINT>>& layoutIndexMap, FrameResource* currentFrameResource)
{
    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
    mCommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
    mCommandList->SetGraphicsRootDescriptorTable(5, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    auto passCB = currentFrameResource->PassCB->Resource();
    mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
    CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    skyTexDescriptor.Offset(layoutIndexMap.at("mSkyTexHeapIndex").first, layoutIndexMap.at("mSkyTexHeapIndex").second);
    mCommandList->SetGraphicsRootDescriptorTable(4, skyTexDescriptor);
    mCommandList->SetPipelineState(mPSOs.at("opaque").Get());
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);
    mCommandList->SetPipelineState(mPSOs.at("skinnedOpaque").Get());
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("SkinnedOpaque"), currentFrameResource);
    mCommandList->SetPipelineState(mPSOs.at("sky").Get());
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Sky"), currentFrameResource);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}