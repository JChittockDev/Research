#include "../../EngineApp.h"

void EngineApp::SssPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mSssRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto sssCBAddress = currentFrameResource->SssCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, sssCBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetPositionGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetAlbedoSpecGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mLighting->GetLightingGpuSrv());

    //float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    //mCommandList->ClearRenderTargetView(mSss->GetSSSCpuRtv(), clearValue, 0, nullptr);
    //mCommandList->OMSetRenderTargets(1, &mSss->GetSSSCpuRtv(), true, nullptr);

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Sss").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

}