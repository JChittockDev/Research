#include "../../EngineApp.h"

void EngineApp::SsaoPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mSsaoRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssaoCBAddress = currentFrameResource->SsaoCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);

    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsao->GetRandomVectorGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSsao->GetAmbientCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsao->GetAmbientCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Ssao").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

}