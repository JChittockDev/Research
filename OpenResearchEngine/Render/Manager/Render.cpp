#include "../../EngineApp.h"

void EngineApp::Render(FrameResource* currentFrameResource)
{
    // Init render command list
    ThrowIfFailed(currentFrameResource->CmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(currentFrameResource->CmdListAlloc.Get(), mPSOs.at("opaque").Get()));
    ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // Add render pass instructions
    DeformationPass(currentFrameResource);
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

    auto& renderItems = mRenderItemLayers.at("Opaque");
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        if (renderItems[i]->AnimationInstance != nullptr)
        {
            mCommandList->SetComputeRootSignature(mSkinnedRootSignature.Get());
            mCommandList->SetPipelineState(mPSOs.at("skinned").Get());
            ComputeSkinning(mCommandList.Get(), renderItems[i], currentFrameResource);
            
            if (renderItems[i]->Geo->Simulation)
            {
                mCommandList->SetComputeRootSignature(mVerletSolverRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("verletSolver").Get());
                ComputeVertletSolver(mCommandList.Get(), renderItems[i], currentFrameResource);
            }
        }
    }
}

void EngineApp::ComputeSkinning(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();
    
    cmdList->SetComputeRootConstantBufferView(0, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->VertexBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->SkinningBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(SkinningInfo));
    
    if (!ri->Geo->Simulation)
    {
        CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cmdList->ResourceBarrier(1, &skinnedBufferBarrier);
    }
    else
    {
        CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        cmdList->ResourceBarrier(1, &skinnedBufferBarrier);
    }

    cmdList->SetComputeRootUnorderedAccessView(3, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    if (!ri->Geo->Simulation)
    {
        CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        cmdList->ResourceBarrier(1, &skinnedBufferBarrier);
    }
    else
    {
        CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        cmdList->ResourceBarrier(1, &skinnedBufferBarrier);
    }
}

void EngineApp::ComputeVertletSolver(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->VertexBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->AdjacencyBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Neighbours));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(3, ri->Geo->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->BaseVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = currentFrameResource->ObjectCB->Resource();
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        auto ri = renderItems[i];

        if (ri->AnimationInstance != nullptr)
        {
            if (!ri->Geo->Simulation)
            {
                cmdList->IASetVertexBuffers(0, 1, &ri->Geo->SkinnedVertexBufferView());
            }
            else
            {
                cmdList->IASetVertexBuffers(0, 1, &ri->Geo->TransformedVertexBufferView());
            }
        }
        else
        {
            cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        }
        
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}