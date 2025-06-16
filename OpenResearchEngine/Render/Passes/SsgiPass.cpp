#include "../../EngineApp.h"

void EngineApp::SsgiPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mSsgiRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssgiCBAddress = currentFrameResource->SsgiCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, ssgiCBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(1, mLighting->GetLightingGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsgi->GetRandomVectorGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSsgi->GetGICpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsgi->GetGICpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Ssgi").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

}