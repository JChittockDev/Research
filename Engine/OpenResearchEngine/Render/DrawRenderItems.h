// Render/DrawRenderItems.h
#pragma once
#include <d3d12.h>
#include <vector>
#include <memory>
#include "../D3D12/D3DUtil.h"
#include "RenderPassConstantBuffers.h"
#include "RenderItem.h"
#include "Resources/MeshAnimationResource.h"

inline void DrawRenderItems(
    ID3D12GraphicsCommandList* cmdList,
    const std::vector<std::shared_ptr<RenderItem>>& renderItems,
    RenderPassConstantBuffers* fr)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = fr->ObjectCB->Resource();
    for (size_t i = 0; i < renderItems.size(); ++i)
    {
        const auto& ri = renderItems[i];
        if (ri->AnimationInstance != nullptr)
        {
            if (!ri->Simulation)
            {
                auto vbv = ri->MeshAnimationResourceInstance->SkinnedVertexBufferView();
                cmdList->IASetVertexBuffers(0, 1, &vbv);
            }
            else
            {
                auto vbv = ri->MeshAnimationResourceInstance->VertexNormalBufferView();
                cmdList->IASetVertexBuffers(0, 1, &vbv);
            }
        }
        else
        {
            auto vbv = ri->Geo->VertexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &vbv);
        }
        auto ibv = ri->Geo->IndexBufferView();
        cmdList->IASetIndexBuffer(&ibv);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->IndexStart, ri->VertexStart, 0);
    }
}
