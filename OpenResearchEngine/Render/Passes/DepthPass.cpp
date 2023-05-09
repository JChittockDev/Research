#include "../../EngineApp.h"

void EngineApp::DepthPass(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, FrameResource* currentFrameResource)
{
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto normalMap = mSsao->NormalMap();
    auto normalMapRtv = mSsao->NormalMapRtv();

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    mCommandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &normalMapRtv, true, &dsv);
    auto passCB = currentFrameResource->PassCB->Resource();
    mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
    mCommandList->SetPipelineState(mPSOs.at("drawNormals").Get());
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);
    mCommandList->SetPipelineState(mPSOs.at("skinnedDrawNormals").Get());
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("SkinnedOpaque"), currentFrameResource);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}