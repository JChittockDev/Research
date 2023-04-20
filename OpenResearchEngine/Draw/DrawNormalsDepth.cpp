#include "../EngineApp.h"

void EngineApp::DrawNormalsAndDepth()
{
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    auto normalMap = mSsao->NormalMap();
    auto normalMapRtv = mSsao->NormalMapRtv();

    // Change to RENDER_TARGET.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the screen normal map and depth buffer.
    float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    mCommandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &normalMapRtv, true, &DepthStencilView());

    // Bind the constant buffer for this pass.
    auto passCB = mCurrFrameResource->PassCB->Resource();
    mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    mCommandList->SetPipelineState(mPSOs["drawNormals"].Get());
    DrawRenderItems(mCommandList.Get(), mRenderItemLayer[(int)RenderLayer::Opaque]);

    mCommandList->SetPipelineState(mPSOs["skinnedDrawNormals"].Get());
    DrawRenderItems(mCommandList.Get(), mRenderItemLayer[(int)RenderLayer::SkinnedOpaque]);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}