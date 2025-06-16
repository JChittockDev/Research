#include "../../EngineApp.h"

void EngineApp::CompositePass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mCompositeRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto objectCBAddress = currentFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(1, mLighting->GetLightingGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsgi->GetGIHorizontalBlurGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Composite").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}