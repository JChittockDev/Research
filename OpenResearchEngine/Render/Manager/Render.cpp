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
    auto& renderItems = mRenderItemLayers.at("Opaque");
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        if (renderItems[i]->AnimationInstance != nullptr)
        {
            mCommandList->SetComputeRootSignature(mBlendRootSignature.Get());
            mCommandList->SetPipelineState(mPSOs.at("blend").Get());
            ComputeBlendshapes(mCommandList.Get(), renderItems[i], currentFrameResource);

            mCommandList->SetComputeRootSignature(mSkinnedRootSignature.Get());
            mCommandList->SetPipelineState(mPSOs.at("skinned").Get());
            ComputeSkinning(mCommandList.Get(), renderItems[i], currentFrameResource);
            
            if (renderItems[i]->Simulation)
            {
                mCommandList->SetComputeRootSignature(mMeshTransferRootSignature.Get());
                mCommandList->SetPipelineState(mPSOs.at("meshTransfer").Get());
                ComputeMeshTransfer(mCommandList.Get(), renderItems[i], currentFrameResource);

                ComputePBD(mCommandList.Get(), renderItems[i], currentFrameResource);

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

void EngineApp::ComputeBlendshapes(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT blendCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(BlendConstants));
    auto blendCB = currentFrameResource->BlendCB->Resource();

    for (UINT i = 0; i < ri->BlendshapeCount; ++i)
    {
        cmdList->SetComputeRoot32BitConstant(0, ri->BlendshapeStart + i, 0);
        cmdList->SetComputeRootConstantBufferView(1, blendCB->GetGPUVirtualAddress() + ri->BlendCBIndex * blendCBByteSize);
        cmdList->SetComputeRootShaderResourceView(2, ri->Geo->BlendshapeBufferGPU->GetGPUVirtualAddress() + (ri->BlendshapeVertexStart + ri->BlendshapeSubsets[i].VertexStart) * sizeof(BlendshapeVertex));

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
        cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

        const UINT threadGroupSizeX = 32;
        const UINT threadGroupSizeY = 1;
        const UINT threadGroupSizeZ = 1;
        cmdList->Dispatch((ri->BlendshapeSubsets[i].VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    }
}

void EngineApp::ComputeSkinning(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();
    
    cmdList->SetComputeRootConstantBufferView(0, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(2, ri->Geo->SkinningBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(SkinningInfo));
    
    if (!ri->Simulation)
    {
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    }
    else
    {
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    }

    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

    // Copy Data
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), ri->Geo->VertexBufferGPU.Get());

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->VertexBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->BlendedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));


    if (!ri->Simulation)
    {
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
    }
    else
    {
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    }
}

void EngineApp::ComputeMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshTransferBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputePBD(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    mCommandList->SetComputeRootSignature(mForceRootSignature.Get());
    mCommandList->SetPipelineState(mPSOs.at("force").Get());
    ComputeForce(mCommandList.Get(), ri, currentFrameResource);
    
    mCommandList->SetComputeRootSignature(mPreSolveRootSignature.Get());
    mCommandList->SetPipelineState(mPSOs.at("preSolve").Get());
    ComputePreSolve(mCommandList.Get(), ri, currentFrameResource);

    for (UINT i = 0; i < 1; ++i)
    {
        mCommandList->SetComputeRootSignature(mStretchConstraintSolveRootSignature.Get());
        mCommandList->SetPipelineState(mPSOs.at("stretchConstraintSolve").Get());
        ComputeStretchConstraintSolve(mCommandList.Get(), ri, currentFrameResource);

        mCommandList->SetComputeRootSignature(mBendingConstraintSolveRootSignature.Get());
        mCommandList->SetPipelineState(mPSOs.at("bendingConstraintSolve").Get());
        ComputeBendingConstraintSolve(mCommandList.Get(), ri, currentFrameResource);

        mCommandList->SetComputeRootSignature(mPostSolveRootSignature.Get());
        mCommandList->SetPipelineState(mPSOs.at("postSolve").Get());
        ComputePostSolve(mCommandList.Get(), ri, currentFrameResource);
    }
}

void EngineApp::ComputeForce(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(1, ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));

    const UINT threadGroupSizeX = 1;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshPreviousSkinnedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputePreSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshForceBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 1;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputeStretchConstraintSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshStretchConstraintIDsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStretchConstraintStart * sizeof(Edge));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshStretchConstraintsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshStretchConstraintStart * sizeof(float));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(4, ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));

    const UINT threadGroupSizeX = 1;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshStretchConstraintCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputeBendingConstraintSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->Geo->SimMeshBendingConstraintIDsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshBendingConstraintStart * sizeof(Edge));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->SimMeshBendingConstraintsBufferGPU->GetGPUVirtualAddress() + ri->SimMeshBendingConstraintStart * sizeof(float));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(4, ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));

    const UINT threadGroupSizeX = 1;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshBendingConstraintCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputePostSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vector3));
    cmdList->SetComputeRootShaderResourceView(1, ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(UINT));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->SimMeshConstraintsVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 1;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->SimMeshVertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

    // Copy Data
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get());
    cmdList->CopyResource(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), ri->Geo->SimMeshNullSolverCountBufferGPU.Get());

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverAccumulationBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->Geo->SimMeshNullSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->SimMeshSolverCountBufferGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputeSimMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->SimMeshSolverVertexBufferGPU->GetGPUVirtualAddress() + ri->SimMeshVertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->MeshTransferBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputeTriangleNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->IndexBufferGPU->GetGPUVirtualAddress() + ri->IndexStart * sizeof(UINT));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(2, ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->TriangleStart * sizeof(TangentNormals));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->TriangleCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void EngineApp::ComputeVertexNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource)
{
    cmdList->SetComputeRootShaderResourceView(0, ri->MeshAnimationResourceInstance->TransformedVertexBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));
    cmdList->SetComputeRootShaderResourceView(1, ri->Geo->TriangleAdjacencyBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Neighbours));
    cmdList->SetComputeRootShaderResourceView(2, ri->MeshAnimationResourceInstance->TriangleNormalBufferGPU->GetGPUVirtualAddress() + ri->TriangleStart * sizeof(TangentNormals));

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    cmdList->SetComputeRootUnorderedAccessView(3, ri->MeshAnimationResourceInstance->VertexNormalBufferGPU->GetGPUVirtualAddress() + ri->VertexStart * sizeof(Vertex));

    const UINT threadGroupSizeX = 64;
    const UINT threadGroupSizeY = 1;
    const UINT threadGroupSizeZ = 1;
    cmdList->Dispatch((ri->VertexCount + threadGroupSizeX - 1) / threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ri->MeshAnimationResourceInstance->VertexNormalBufferGPU.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
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
            if (!ri->Simulation)
            {
                cmdList->IASetVertexBuffers(0, 1, &ri->MeshAnimationResourceInstance->SkinnedVertexBufferView());
            }
            else
            {
                cmdList->IASetVertexBuffers(0, 1, &ri->MeshAnimationResourceInstance->VertexNormalBufferView());
            }
        }
        else
        {
            cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        }
        
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->IndexStart, ri->VertexStart, 0);
    }
}