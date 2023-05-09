#include "RenderCore.h"

void RenderCore::ShadowPass(const std::shared_ptr<DynamicLights>& lights, FrameResource* currentFrameResource)
{
    for (int i = 0; i < lights->LightTransforms.size(); i++)
    {
        commandList->RSSetViewports(1, &shadowMaps[i]->Viewport());
        commandList->RSSetScissorRects(1, &shadowMaps[i]->ScissorRect());
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        commandList->ClearDepthStencilView(shadowMaps[i]->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        commandList->OMSetRenderTargets(0, nullptr, false, &shadowMaps[i]->Dsv());
        UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
        auto passCB = currentFrameResource->PassCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + (i + 1) * passCBByteSize;
        commandList->SetGraphicsRootConstantBufferView(2, passCBAddress);
        commandList->SetPipelineState(psos->at("shadow_opaque").Get());
        SetRenderItems(commandList, renderItemLayers->at("Opaque"), currentFrameResource);
        commandList->SetPipelineState(psos->at("skinnedShadow_opaque").Get());
        SetRenderItems(commandList, renderItemLayers->at("SkinnedOpaque"), currentFrameResource);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMaps[i]->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
    }
}