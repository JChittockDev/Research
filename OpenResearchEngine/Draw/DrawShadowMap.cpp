#include "../EngineApp.h"

void EngineApp::DrawSceneToShadowMap()
{
    for (int i = 0; i < dynamicLights.LightTransforms.size(); i++)
    {
        mCommandList->RSSetViewports(1, &mShadowMaps[i]->Viewport());
        mCommandList->RSSetScissorRects(1, &mShadowMaps[i]->ScissorRect());

        // Change to DEPTH_WRITE.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        // Clear the back buffer and depth buffer.
        mCommandList->ClearDepthStencilView(mShadowMaps[i]->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // Specify the buffers we are going to render to.
        mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMaps[i]->Dsv());

        // Bind the pass constant buffer for the shadow map pass.
        UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
        auto passCB = mCurrFrameResource->PassCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + (i + 1) * passCBByteSize;
        mCommandList->SetGraphicsRootConstantBufferView(2, passCBAddress);

        mCommandList->SetPipelineState(mPSOs["shadow_opaque"].Get());
        DrawRenderItems(mCommandList.Get(), mRenderItemLayer[(int)RenderLayer::Opaque]);

        mCommandList->SetPipelineState(mPSOs["skinnedShadow_opaque"].Get());
        DrawRenderItems(mCommandList.Get(), mRenderItemLayer[(int)RenderLayer::SkinnedOpaque]);

        // Change back to GENERIC_READ so we can read the texture in a shader.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
    }
}