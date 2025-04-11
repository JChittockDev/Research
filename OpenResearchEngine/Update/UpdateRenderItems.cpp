#include "../EngineApp.h"

void EngineApp::UpdateObjectCBs(const GameTimer& gt)
{
    ImGui::SeparatorText("Objects");

    auto currObjectCB = mCurrFrameResource->ObjectCB.get();

    for (auto& e : mMeshRenderItemMap)
    {
        std::vector<std::shared_ptr<RenderItem>>& renderItems = e.second;
        for (int i = 0; i < renderItems.size(); i++)
        {
            auto& renderItem = renderItems[i];
            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&renderItem->World);

            std::string line = e.first + " (Instance " + std::to_string(i) + ")";
            // Create a collapsible tree node for each RenderItem
            if (ImGui::TreeNode(line.c_str()))
            {
                // Load the World matrix and decompose it into position, rotation, and scale
                DirectX::XMVECTOR scale, rotationQuat, translation;
                DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, world);

                DirectX::XMFLOAT3 position, rotationEuler, scaleValues;
                DirectX::XMStoreFloat3(&position, translation);
                DirectX::XMStoreFloat3(&scaleValues, scale);

                // Convert quaternion rotation to Euler angles for display
                DirectX::XMFLOAT4 rotationQuatValues;
                DirectX::XMStoreFloat4(&rotationQuatValues, rotationQuat);
                rotationEuler = Math::QuaternionToEuler(rotationQuatValues);

                // Display position, rotation (Euler angles), and scale as read-only text
                ImGui::DragFloat3("Position", &position.x);
                ImGui::DragFloat3("Rotation (Euler)", &rotationEuler.x);
                ImGui::DragFloat3("Scale", &scaleValues.x);

                // If any values have changed, recompose the World matrix
                DirectX::XMVECTOR newScale = DirectX::XMLoadFloat3(&scaleValues);
                DirectX::XMVECTOR newRotation = Math::EulerToQuaternion(rotationEuler);
                DirectX::XMVECTOR newTranslation = DirectX::XMLoadFloat3(&position);

                // Create a new World matrix from modified position, rotation, and scale
                DirectX::XMMATRIX newWorld = DirectX::XMMatrixScalingFromVector(newScale) * DirectX::XMMatrixRotationQuaternion(newRotation) * DirectX::XMMatrixTranslationFromVector(newTranslation);

                // Update the World matrix in RenderItem
                DirectX::XMStoreFloat4x4(&renderItem->World, newWorld);

                // Flag the item as needing an update
                renderItem->NumFramesDirty = gNumFrameResources;

                // Close the tree node
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

                // Next FrameResource need to be updated too.
                renderItems[i]->NumFramesDirty--;
            }
        }
    }
}
