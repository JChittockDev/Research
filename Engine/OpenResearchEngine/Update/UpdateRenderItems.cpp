#include "UpdateFunctions.h"
#include "../Common/Structures.h"
#include "../Common/Math.h"
#include "../Render/Resources/FrameResource.h"
#include "../Assets/AssetManager.h"
#include "../Utilities/GameTimer.h"
#include <imgui.h>

void UpdateObjectCBs(const GameTimer& gt, AssetManager& assets, FrameResource* fr)
{
    ImGui::SeparatorText("Objects");

    auto currObjectCB = fr->ObjectCB.get();

    for (auto& e : assets.mMeshRenderItemMap)
    {
        std::vector<std::shared_ptr<RenderItem>>& renderItems = e.second;
        for (int i = 0; i < (int)renderItems.size(); i++)
        {
            auto& renderItem = renderItems[i];
            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&renderItem->World);

            std::string line = e.first + " (Instance " + std::to_string(i) + ")";
            if (ImGui::TreeNode(line.c_str()))
            {
                DirectX::XMVECTOR scale, rotationQuat, translation;
                DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, world);

                DirectX::XMFLOAT3 position, rotationEuler, scaleValues;
                DirectX::XMStoreFloat3(&position, translation);
                DirectX::XMStoreFloat3(&scaleValues, scale);

                DirectX::XMFLOAT4 rotationQuatValues;
                DirectX::XMStoreFloat4(&rotationQuatValues, rotationQuat);
                rotationEuler = Math::QuaternionToEuler(rotationQuatValues);

                ImGui::DragFloat3("Position", &position.x);
                ImGui::DragFloat3("Rotation (Euler)", &rotationEuler.x);
                ImGui::DragFloat3("Scale", &scaleValues.x);

                DirectX::XMVECTOR newScale = DirectX::XMLoadFloat3(&scaleValues);
                DirectX::XMVECTOR newRotation = Math::EulerToQuaternion(rotationEuler);
                DirectX::XMVECTOR newTranslation = DirectX::XMLoadFloat3(&position);

                DirectX::XMMATRIX newWorld = DirectX::XMMatrixScalingFromVector(newScale) * DirectX::XMMatrixRotationQuaternion(newRotation) * DirectX::XMMatrixTranslationFromVector(newTranslation);
                DirectX::XMStoreFloat4x4(&renderItem->World, newWorld);

                renderItem->NumFramesDirty = gNumFrameResources;

                ImGui::TreePop();
            }

            if (renderItems[i]->NumFramesDirty > 0)
            {
                DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&renderItems[i]->TexTransform);

                ObjectConstants objConstants;
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
                XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
                objConstants.MaterialIndex = renderItems[i]->Mat->MaterialIndex;

                currObjectCB->CopyData(renderItems[i]->ObjCBIndex, objConstants);
                renderItems[i]->NumFramesDirty--;
            }
        }
    }
}
