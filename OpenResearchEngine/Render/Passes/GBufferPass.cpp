#include "../../EngineApp.h"

void EngineApp::GBufferPass(FrameResource* currentFrameResource)
{
    // Set the Geometry Pass Root Signature
    mCommandList->SetGraphicsRootSignature(mGBufferRootSignature.Get());

    // Set Viewport and Scissor Rects
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    // Transition G-Buffer textures to RENDER_TARGET state
    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetPosition().Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetNormal().Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetAlbedoSpec().Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear G-Buffer render targets
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    mCommandList->ClearRenderTargetView(mGBuffer->GetPositionRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetNormalRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetAlbedoSpecRtv(), clearColor, 0, nullptr);

    // Set render targets for the G-Buffer
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
        mGBuffer->GetPositionRtv(),
        mGBuffer->GetNormalRtv(),
        mGBuffer->GetAlbedoSpecRtv()
    };

    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    mCommandList->OMSetRenderTargets(3, rtvs, false, &DepthStencilView());

    // Set root parameters
    auto objectCBAddress = currentFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto passCBAddress = currentFrameResource->PassCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress = currentFrameResource->MaterialBuffer->Resource()->GetGPUVirtualAddress();
    CD3DX12_GPU_DESCRIPTOR_HANDLE textureDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    mCommandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);
    mCommandList->SetGraphicsRootShaderResourceView(2, matBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(3, textureDescriptor);

    // Set Pipeline State for G-Buffer Pass
    mCommandList->SetPipelineState(mPSOs.at("GBuffer").Get());

    // Draw all objects in the scene
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);

    // Transition G-Buffer textures back to GENERIC_READ for later use
    //CD3DX12_RESOURCE_BARRIER postBarriers[] = {
    //    CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetPosition().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
    //    CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetNormal().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
    //    CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetAlbedoSpec().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)
    //};
    //mCommandList->ResourceBarrier(3, postBarriers);
}