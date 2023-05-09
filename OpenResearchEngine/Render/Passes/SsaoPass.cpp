#include "../../EngineApp.h"

void EngineApp::SsaoPass(int blurCount, FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mSsaoRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSsao->AmbientMap(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mCommandList->ClearRenderTargetView(mSsao->AmbientMapRtv(), clearValue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &mSsao->AmbientMapRtv(), true, nullptr);
    auto ssaoCBAddress = currentFrameResource->SsaoCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);
    mCommandList->SetGraphicsRoot32BitConstant(1, 0, 0);
    mCommandList->SetGraphicsRootDescriptorTable(2, mSsao->NormalMapSrv());
    mCommandList->SetGraphicsRootDescriptorTable(3, mSsao->RandomVectorMapSrv());
    mCommandList->SetPipelineState(mSsao->GetPSO());
    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSsao->AmbientMap(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
    mSsao->BlurAmbientMap(mCommandList.Get(), currentFrameResource, blurCount);
}