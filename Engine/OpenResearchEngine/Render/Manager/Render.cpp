#include "../../EngineApp.h"
#include <unordered_set>

void EngineApp::Render(FrameResource* currentFrameResource)
{
    // Init render command list
    ThrowIfFailed(currentFrameResource->CmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(currentFrameResource->CmdListAlloc.Get(), mPSOs.at("GBuffer").Get()));
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderPassSrvHeap.Get()};
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


    mOnnxModelResource->Evaluate(std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f});


    // Add render pass instructions
    DeformationPass(currentFrameResource);
    ShadowPass(dynamicLights, currentFrameResource);
    GBufferPass(currentFrameResource);
    SsaoPass(currentFrameResource);
    SsaoBlurPass(currentFrameResource);
    RadiancePass(dynamicLights, currentFrameResource);
    SssPass(currentFrameResource);
    SssBlurPass(currentFrameResource);
    LightingPass(currentFrameResource);
    SsgiPass(currentFrameResource);
    SsgiBlurPass(currentFrameResource);
    CompositePass(currentFrameResource);
}

void EngineApp::DeformationPass(FrameResource* currentFrameResource)
{
    auto& renderItems = mRenderItemLayers.at("Opaque");

    // Group by MeshInstance so each instance's graph runs once per frame
    // (multiple subsets share the same MeshInstance)
    std::unordered_set<MeshInstance*> processed;

    for (auto& ri : renderItems) {
        if (!ri->Instance || ri->Instance->Graph().IsEmpty()) continue;
        if (processed.count(ri->Instance)) continue;
        processed.insert(ri->Instance);

        // Execute all deformers for all subsets of this instance
        for (auto& [subsetName, args] : ri->Instance->Asset()->DrawArgs) {
            DeformContext ctx{};
            ctx.MeshAsset  = ri->Instance->Asset();
            ctx.Frame      = currentFrameResource;
            ctx.SubsetName = &subsetName;
            ri->Instance->Graph().Execute(mCommandList.Get(), ctx);
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

        if (ri->Instance != nullptr)
        {
            ID3D12Resource* vb = ri->Instance->FinalVertexBuffer();
            D3D12_VERTEX_BUFFER_VIEW vbv = ri->Instance->Asset()->MakeVertexBufferView(vb);
            cmdList->IASetVertexBuffers(0, 1, &vbv);
            cmdList->IASetIndexBuffer(&ri->Instance->Asset()->IndexBufferView());
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
            cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
            const auto& args = ri->Instance->Asset()->DrawArgs.at(ri->SubsetName);
            cmdList->DrawIndexedInstanced(args.IndexCount, 1, args.IndexStart, args.VertexStart, 0);
        }
    }
}
