#include "../../EngineApp.h"

void EngineApp::SsaoBlurPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mEdgeBlurRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssaoVerticalBlurCBAddress = currentFrameResource->SsaoVerticalBlurCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, ssaoVerticalBlurCBAddress);

    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsao->GetDepthGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsao->GetAmbientGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSsao->GetAmbientVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsao->GetAmbientVerticalBlurCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("EdgeBlur").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    //////////////////////////////////////////////////////////////////////////////////////////////////

    mCommandList->SetGraphicsRootSignature(mEdgeBlurRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssaoHorizontalBlurCBAddress = currentFrameResource->SsaoHorizontalBlurCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, ssaoHorizontalBlurCBAddress);

    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsao->GetDepthGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsao->GetAmbientVerticalBlurGpuSrv());

    mCommandList->ClearRenderTargetView(mSsao->GetAmbientHorizontalBlurCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsao->GetAmbientHorizontalBlurCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("EdgeBlur").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);


}