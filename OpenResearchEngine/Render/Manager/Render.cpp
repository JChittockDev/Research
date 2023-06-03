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
                mCommandList->SetComputeRootSignature(mMeshTransferRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("meshTransfer").Get());
                ComputeMeshTransfer(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mPreSolveRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("preSolve").Get());
                ComputePreSolve(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mVerletSolverRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("verletSolver").Get());
                ComputeVertletSolver(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mPostSolveRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("postSolve").Get());
                ComputePostSolve(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mSimMeshTransferRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("simMeshTransfer").Get());
                ComputeSimMeshTransfer(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mTriangleNormalRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("triangleNormal").Get());
                ComputeTriangleNormals(mCommandList.Get(), renderItems[i], currentFrameResource);

                mCommandList->SetComputeRootSignature(mVertexNormalRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("vertexNormal").Get());
                ComputeVertexNormals(mCommandList.Get(), renderItems[i], currentFrameResource);
            }
        }
    }
}

void EngineApp::ComputeSkinning(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();
    
    cmdList->SetComputeRootConstantBufferView(0, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->VertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->SkinningBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(SkinningInfo));
    
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

    cmdList->SetComputeRootUnorderedAccessView(3, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    //cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

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

void EngineApp::ComputeMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshTransferBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(UINT));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::ComputePreSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshSpringTransformBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Spring));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshTransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->SimMeshTransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshTransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}


void EngineApp::ComputeVertletSolver(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshRestConstraintBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(RestConstraint));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshVertexAdjacencyBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Neighbours));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshTransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->SimMeshTransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshTransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::ComputePostSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshTransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshSpringTransformBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->SimMeshSpringTransformBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Spring));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshSpringTransformBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::ComputeSimMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshTransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->MeshTransferBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(UINT));

    CD3DX12_RESOURCE_BARRIER transformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &transformedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::ComputeTriangleNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->IndexBufferGPU->GetGPUVirtualAddress() + ri->StartIndexLocation * sizeof(UINT));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(2, ri->Geo->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->StartTriangleLocation * sizeof(TangentNormals));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->TriangleCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &resetTransformedBufferBarrier);
}

void EngineApp::ComputeVertexNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->TriangleAdjacencyBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Neighbours));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->StartTriangleLocation * sizeof(TangentNormals));

    CD3DX12_RESOURCE_BARRIER skinnedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &skinnedBufferBarrier);

    cmdList->SetComputeRootUnorderedAccessView(3, ri->Geo->VertexNormalBufferGPU->GetGPUVirtualAddress() + ri->StartVertexLocation * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    CD3DX12_RESOURCE_BARRIER resetTransformedBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
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
                cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexNormalBufferView());
            }
        }
        else
        {
            cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        }
        
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->StartVertexLocation, 0);
    }
}