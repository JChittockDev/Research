#include "../EngineApp.h"

DirectX::XMFLOAT3 QuaternionToEuler(const DirectX::XMFLOAT4& quat)
{
    DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&quat);
    DirectX::XMFLOAT3 euler;
    euler.x = atan2(2.0f * (quat.w * quat.x + quat.y * quat.z), 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y));
    euler.y = asin(2.0f * (quat.w * quat.y - quat.z * quat.x));
    euler.z = atan2(2.0f * (quat.w * quat.z + quat.x * quat.y), 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z));
    return euler;
}

DirectX::XMVECTOR EulerToQuaternion(const DirectX::XMFLOAT3& euler)
{
    using namespace DirectX;
    return XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
}

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
                rotationEuler = QuaternionToEuler(rotationQuatValues);

                // Display position, rotation (Euler angles), and scale as read-only text
                ImGui::InputFloat3("Position", &position.x);
                ImGui::InputFloat3("Rotation (Euler)", &rotationEuler.x);
                ImGui::InputFloat3("Scale", &scaleValues.x);

                // If any values have changed, recompose the World matrix
                DirectX::XMVECTOR newScale = DirectX::XMLoadFloat3(&scaleValues);
                DirectX::XMVECTOR newRotation = EulerToQuaternion(rotationEuler);
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
