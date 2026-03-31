#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include "Resources/RenderItem.h"
#include "Resources/FrameResource.h"
#include "../D3D12/D3DUtil.h"
#include "../Common/Structures.h"

inline void DrawRenderItems(
    ID3D12GraphicsCommandList* cmdList,
    const std::vector<std::shared_ptr<RenderItem>>& renderItems,
    FrameResource* fr)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = fr->ObjectCB->Resource();
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
