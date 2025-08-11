#include "../../EngineApp.h"

void EngineApp::LightingPass(FrameResource* currentFrameResource)
{
    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    mCommandList->SetGraphicsRootSignature(mLightingRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto objectCBAddress = currentFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto passCBAddress = currentFrameResource->PassCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress = currentFrameResource->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);
    mCommandList->SetGraphicsRootShaderResourceView(2, matBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetPositionGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(4, mSss->GetSSSHorizontalBlurGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(5, mRadianceResources->GetStartGpuSrv());

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mLighting->GetLightingCpuRtv(), clearColor, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mLighting->GetLightingCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Lighting").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);
}