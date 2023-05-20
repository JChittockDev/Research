#include "../../EngineApp.h"

void EngineApp::Render(FrameResource* currentFrameResource)
{
    // Init render command list
    ThrowIfFailed(currentFrameResource->CmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(currentFrameResource->CmdListAlloc.Get(), mPSOs.at("opaque").Get()));
    ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // Add render pass instructions
    //DeformationPass(currentFrameResource);
    ShadowPass(dynamicLights, currentFrameResource);
    DepthPass(DepthStencilView(), currentFrameResource);
    SsaoPass(2, currentFrameResource);
    DiffusePass(mLayoutIndicies, currentFrameResource);

    ThrowIfFailed(mCommandList->Close());
}

void EngineApp::DeformationPass(FrameResource* currentFrameResource)
{
    mCommandList->SetComputeRootSignature(mSkinnedRootSignature.Get());
    mCommandList->SetPipelineState(mPSOs.at("skinned").Get());
    ComputeSkinning(mCommandList.Get(), mRenderItemLayers.at("Opaque"), currentFrameResource);
}

void EngineApp::ComputeSkinning(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource)
{
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        auto ri = renderItems[i];

        if (ri->AnimationInstance != nullptr)
        {
            cmdList->SetComputeRootConstantBufferView(0, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
            cmdList->SetComputeRootShaderResourceView(1, ri->Geo->VertexBufferGPU->GetGPUVirtualAddress());
            cmdList->SetComputeRootShaderResourceView(2, ri->Geo->SkinningBufferGPU->GetGPUVirtualAddress());

            CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->ResourceBarrier(1, &skinnedBufferBarrier);

            cmdList->SetComputeRootUnorderedAccessView(3, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress());

            const UINT threadGroupSizeX = 64;
            const UINT threadGroupSizeY = 1;
            const UINT threadGroupSizeZ = 1;
            cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

            skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &skinnedBufferBarrier);
        }
    }
}

void EngineApp::SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = currentFrameResource->ObjectCB->Resource();
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        auto ri = renderItems[i];
        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);

        if (ri->AnimationInstance != nullptr)
        {
            cmdList->SetGraphicsRootShaderResourceView(5, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress());
        }
        else
        {
            cmdList->SetGraphicsRootShaderResourceView(5, ri->Geo->VertexBufferGPU->GetGPUVirtualAddress());
        }
    }
}