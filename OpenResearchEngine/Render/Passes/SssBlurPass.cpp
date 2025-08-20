#include "../../EngineApp.h"

void EngineApp::SssBlurPass(FrameResource* currentFrameResource)
{
    // First pass - Vertical Blur
    mCommandList->SetGraphicsRootSignature(mPoissonBlurRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSS().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    auto sssVerticalBlurCBAddress = currentFrameResource->SssBlurCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, sssVerticalBlurCBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(1, mSss->GetDepthGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSss->GetSSSGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSss->GetSSSBlurCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSss->GetSSSBlurCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("PoissonBlur").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSss->GetSSS().Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}