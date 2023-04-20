#include "../EngineApp.h"

void EngineApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto skinnedCB = mCurrFrameResource->SkinnedCB->Resource();

    // For each render item...
    for (size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

        cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

        if (ri->AnimationInstance != nullptr)
        {
            D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize;
            cmdList->SetGraphicsRootConstantBufferView(1, skinnedCBAddress);
        }
        else
        {
            cmdList->SetGraphicsRootConstantBufferView(1, 0);
        }

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}