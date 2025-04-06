#include "../../EngineApp.h"

void EngineApp::LightingPass(const std::unordered_map<std::string, std::pair<INT, UINT>>& layoutIndexMap, FrameResource* currentFrameResource)
{
    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    mCommandList->SetGraphicsRootSignature(mLightingRootSignature.Get());
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    mCommandList->SetGraphicsRootConstantBufferView(1, currentFrameResource->PassCB->Resource()->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE textureDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    textureDescriptor.Offset(layoutIndexMap.at("mSkyTexHeapIndex").first, layoutIndexMap.at("mSkyTexHeapIndex").second);
    mCommandList->SetGraphicsRootDescriptorTable(2, textureDescriptor);

    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetPosition().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetNormal().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    //mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGBuffer->GetAlbedoSpec().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    ID3D12DescriptorHeap* descriptorHeaps[] = {gBufferSrvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE textureDescriptor2(gBufferSrvHeap->GetGPUDescriptorHandleForHeapStart());
    mCommandList->SetGraphicsRootDescriptorTable(3, textureDescriptor2);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);

    mCommandList->SetPipelineState(mPSOs.at("Lighting").Get());

    mCommandList->IASetVertexBuffers(0, 0, nullptr);
    mCommandList->IASetIndexBuffer(nullptr);
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawInstanced(6, 1, 0, 0);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}