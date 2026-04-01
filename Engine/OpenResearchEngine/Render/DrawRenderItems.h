#pragma once
#include <d3d12.h>
#include <vector>
#include <memory>
#include "../D3D12/D3DUtil.h"
#include "Resources/RenderItem.h"
#include "Resources/FrameResource.h"
#include "../Common/Structures.h"

// Binds per-object state and issues draw calls for a list of render items.
inline void DrawRenderItems(
    ID3D12GraphicsCommandList* cmdList,
    const std::vector<std::shared_ptr<RenderItem>>& renderItems,
    FrameResource* fr)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    auto objectCB = fr->ObjectCB->Resource();

    for (const auto& ri : renderItems)
    {
        if (ri->AnimationInstance != nullptr)
        {
            if (!ri->Simulation)
                cmdList->IASetVertexBuffers(0, 1, &ri->MeshAnimationResourceInstance->SkinnedVertexBufferView());
            else
                cmdList->IASetVertexBuffers(0, 1, &ri->MeshAnimationResourceInstance->VertexNormalBufferView());
        }
        else
        {
            cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        }

        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
        cmdList->SetGraphicsRootConstantBufferView(
            0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->IndexStart, ri->VertexStart, 0);
    }
}
