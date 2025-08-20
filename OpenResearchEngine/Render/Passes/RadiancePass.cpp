#include "../../EngineApp.h"

void EngineApp::RadiancePass(const DynamicLights& lights, FrameResource* currentFrameResource)
{
    mCommandList->SetGraphicsRootSignature(mRadianceRootSignature.Get());
    
    auto objectCBAddress = currentFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();
    auto radianceCBAddress = currentFrameResource->RadianceCB->Resource()->GetGPUVirtualAddress();
    auto matBAddress = currentFrameResource->MaterialBuffer->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
    mCommandList->SetGraphicsRootShaderResourceView(2, matBAddress);
    mCommandList->SetGraphicsRootDescriptorTable(3, mGBuffer->GetPositionGpuSrv());
    
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScreenScissorRect);

    for (int i = 0; i < lights.LightTransforms.size(); i++)
    {
        UINT radianceCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(RadianceConstants));
        auto radianceCB = currentFrameResource->RadianceCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS radianceCBAddress = radianceCB->GetGPUVirtualAddress() + (i) * radianceCBByteSize;
        mCommandList->SetGraphicsRootConstantBufferView(1, radianceCBAddress);

        mCommandList->SetGraphicsRootDescriptorTable(4, mShadowResources->shadowMaps[i]->Srv());

        float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        mCommandList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetDiffuseReflectanceCpuRtv(), clearColor, 0, nullptr);
        mCommandList->ClearRenderTargetView(mRadianceResources->radianceMaps[i]->GetSpecularReflectanceCpuRtv(), clearColor, 0, nullptr);

        // Set render targets for the G-Buffer
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
            mRadianceResources->radianceMaps[i]->GetDiffuseReflectanceCpuRtv(),
            mRadianceResources->radianceMaps[i]->GetSpecularReflectanceCpuRtv()
        };

        mCommandList->OMSetRenderTargets(2, rtvs, false, nullptr);
        mCommandList->SetPipelineState(mPSOs.at("Radiance").Get());
        mCommandList->IASetVertexBuffers(0, 0, nullptr);
        mCommandList->IASetIndexBuffer(nullptr);
        mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mCommandList->DrawInstanced(6, 1, 0, 0);
    }
}