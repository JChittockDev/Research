#include "../../EngineApp.h"

void EngineApp::GBufferPass(FrameResource* currentFrameResource)
{
    // Set the Geometry Pass Root Signature
    mCommandList->SetGraphicsRootSignature(mGBufferRootSignature.Get());

    // Set Viewport and Scissor Rects
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    // Clear G-Buffer render targets
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mGBuffer->GetPositionCpuRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetNormalCpuRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetAlbedoSpecCpuRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetReflectionCpuRtv(), clearColor, 0, nullptr);
    mCommandList->ClearRenderTargetView(mGBuffer->GetMaterialIdCpuRtv(), clearColor, 0, nullptr);

    // Set render targets for the G-Buffer
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
        mGBuffer->GetPositionCpuRtv(),
        mGBuffer->GetNormalCpuRtv(),
        mGBuffer->GetAlbedoSpecCpuRtv(),
        mGBuffer->GetReflectionCpuRtv(),
        mGBuffer->GetMaterialIdCpuRtv()
    };

    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    mCommandList->OMSetRenderTargets(5, rtvs, false, &DepthStencilView());

    // Set root parameters
    auto objectCBAddress = currentFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto passCBAddress = currentFrameResource->PassCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress = currentFrameResource->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);
    mCommandList->SetGraphicsRootShaderResourceView(2, matBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(3, mRenderTextures->GetStartGpuSrv());

    // Set Pipeline State for G-Buffer Pass
    mCommandList->SetPipelineState(mPSOs.at("GBuffer").Get());

    // Draw all objects in the scene
    SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);
}