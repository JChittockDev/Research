#include "../../EngineApp.h"

void EngineApp::SssPass(FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mSssRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    auto matBAddress = currentFrameResource->MaterialBuffer->Resource()->GetGPUVirtualAddress();
    auto sssCBAddress = currentFrameResource->SssCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, sssCBAddress);
    mCommandList->SetGraphicsRootShaderResourceView(1, matBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(2, mGBuffer->GetPositionGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetAlbedoSpecGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(4, mGBuffer->GetTangentGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(5, mGBuffer->GetMaterialIdGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(6, mSss->GetRandomVectorGpuSrv());
    mCommandList->SetGraphicsRootDescriptorTable(7, mRadianceResources->GetStartGpuSrv());

    float clearValue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSss->GetSSSCpuRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSss->GetSSSCpuRtv(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Sss").Get());
   
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);
}