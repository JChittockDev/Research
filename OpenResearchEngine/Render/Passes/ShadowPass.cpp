#include "../../EngineApp.h"

void EngineApp::ShadowPass(const DynamicLights& lights, FrameResource* currentFrameResource)
{
    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    mCommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());
    mCommandList->SetGraphicsRootDescriptorTable(4, mNullSrv);
    mCommandList->SetGraphicsRootDescriptorTable(5, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    for (int i = 0; i < lights.LightTransforms.size(); i++)
    {
        mCommandList->RSSetViewports(1, &mShadowMaps[i]->Viewport());
        mCommandList->RSSetScissorRects(1, &mShadowMaps[i]->ScissorRect());
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        mCommandList->ClearDepthStencilView(mShadowMaps[i]->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMaps[i]->Dsv());
        UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
        auto passCB = currentFrameResource->PassCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + (i + 1) * passCBByteSize;
        mCommandList->SetGraphicsRootConstantBufferView(2, passCBAddress);
        mCommandList->SetPipelineState(mPSOs.at("shadow_opaque").Get());
        SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);
        mCommandList->SetPipelineState(mPSOs.at("skinnedShadow_opaque").Get());
        SetRenderItems(mCommandList.Get(), mRenderItemLayers.at("SkinnedOpaque"), currentFrameResource);
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
    }
}