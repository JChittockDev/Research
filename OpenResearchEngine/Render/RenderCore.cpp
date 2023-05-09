#include "RenderCore.h"

void RenderCore::Initialize(
    ID3D12Resource* inCBB,
    const D3D12_CPU_DESCRIPTOR_HANDLE& inCbbv,
    const D3D12_CPU_DESCRIPTOR_HANDLE& inDsv,
    const CD3DX12_GPU_DESCRIPTOR_HANDLE& inNullSrv,
    ID3D12GraphicsCommandList* inCommandList,
    ID3D12RootSignature* inRootSignature,
    ID3D12RootSignature* inSsaoRootSignature,
    ID3D12DescriptorHeap* inSrvDescriptorHeap,
    const std::unordered_map<std::string, std::pair<INT, UINT>>& inLayoutIndicies,
    const std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>& inPsos,
    const std::vector<std::shared_ptr<RenderItem>>& inRenderItems,
    const std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>& inRenderItemLayers,
    const DynamicLights& inLights)
{
    cBB = inCBB;
    cbbv = inCbbv;
    dsv = inDsv;
    nullSrv = inNullSrv;
    commandList = inCommandList;
    rootSignature = inRootSignature;
    ssaoRootSignature = inSsaoRootSignature;
    srvDescriptorHeap = inSrvDescriptorHeap;
    layoutIndicies = std::make_shared<std::unordered_map<std::string, std::pair<INT, UINT>>>(inLayoutIndicies);
    psos = std::make_shared<std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>>(inPsos);
    renderItems = std::make_shared<std::vector<std::shared_ptr<RenderItem>>>(inRenderItems);
    renderItemLayers = std::make_shared<std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>>(inRenderItemLayers);
    lights = std::make_shared<DynamicLights>(inLights);
}

void RenderCore::Render(FrameResource* currentFrameResource)
{
    ThrowIfFailed(currentFrameResource->CmdListAlloc->Reset());
    ThrowIfFailed(commandList->Reset(currentFrameResource->CmdListAlloc.Get(), psos->at("opaque").Get()));

    ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootSignature(rootSignature);

    auto matBuffer = currentFrameResource->MaterialBuffer->Resource();
    commandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(4, nullSrv);
    commandList->SetGraphicsRootDescriptorTable(5, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    ShadowPass(lights, currentFrameResource);
    DepthPass(dsv, currentFrameResource);
    commandList->SetGraphicsRootSignature(ssaoRootSignature);
    SsaoPass(2, currentFrameResource);

    commandList->SetGraphicsRootSignature(rootSignature);
    matBuffer = currentFrameResource->MaterialBuffer->Resource();
    commandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());

    DiffusePass(layoutIndicies, currentFrameResource);

    ThrowIfFailed(commandList->Close());
}

void RenderCore::SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));
    auto objectCB = currentFrameResource->ObjectCB->Resource();
    auto skinnedCB = currentFrameResource->SkinnedCB->Resource();

    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        auto ri = renderItems[i];
        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);

        if (ri->AnimationInstance != nullptr)
        {
            cmdList->SetGraphicsRootConstantBufferView(1, skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize);
        }
        else
        {
            cmdList->SetGraphicsRootConstantBufferView(1, 0);
        }

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}