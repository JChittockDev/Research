#include "../../EngineApp.h"

void EngineApp::SsgiBlurPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mColorEdgeBlurRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssgiVerticalBlurCBAddress = currentFrameResource->SsgiVerticalBlurCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, ssgiVerticalBlurCBAddress);

    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIGpuSrv());

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSsgi->GetGIVerticalBlurCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsgi->GetGIVerticalBlurCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("ColorEdgeBlur").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    mCommandList->SetGraphicsRootSignature(mColorEdgeBlurRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto ssgiHorizontalBlurCBAddress = currentFrameResource->SsgiHorizontalBlurCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, ssgiHorizontalBlurCBAddress);

    mCommandList->SetGraphicsRootDescriptorTable(1, mGBuffer->GetNormalGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsgi->GetDepthGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsgi->GetGIVerticalBlurGpuSrv());

    mCommandList->ClearRenderTargetView(mSsgi->GetGIHorizontalBlurCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsgi->GetGIHorizontalBlurCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("ColorEdgeBlur").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);
}