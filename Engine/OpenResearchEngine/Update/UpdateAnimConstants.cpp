#include "../EngineApp.h"

std::vector<Vector4> GetWeights(std::map<std::string, std::vector<float>>& weights)
{
    std::vector<Vector4> output;
    for (auto subset : weights)
    {
        for (int i = 0; i < subset.second.size(); i++)
        {
            Vector4 packedWeight;
            packedWeight.x = subset.second[i];
            output.push_back(packedWeight);
        }
    }
    return output;
}

void EngineApp::UpdateAnimCBs(const GameTimer& gt)
{
    ImGui::SeparatorText("Animation Controllers");

    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();
    auto currBlendCB = mCurrFrameResource->BlendCB.get();

    int skinningIndex = 0;
    std::map<std::string, SkinnedConstants> constants;
    for (auto controller : mSkinningControllers)
    {
        bool animCheck = mSkinningControllers.at(controller.first)->animation == nullptr || mSkinningControllers.at(controller.first)->animation->TransformAnimNodes.size() == 0;
        if (!animCheck)
        {
            mSkinningControllers.at(controller.first)->UpdateSkinning(gt.DeltaTime());
        }
        SkinnedConstants skinnedConstants;
        std::copy(std::begin(mSkinningControllers.at(controller.first)->transforms), std::end(mSkinningControllers.at(controller.first)->transforms), &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(skinningIndex, skinnedConstants);
        constants[controller.first] = skinnedConstants;
        skinningIndex++;
    }

    if (ImGui::TreeNode("Skinning Controllers"))
    {
        int imguiSkinningIndex = 0;
        for (auto controller : mSkinningControllers)
        {
            SkinnedConstants skinnedConstants = constants[controller.first];
            if (ImGui::TreeNode(controller.first.c_str()))
            {
                for (int i = 0; i < mSkinningControllers.at(controller.first)->transforms.size(); i++)
                {
                    std::string transform_name = "Transform " + std::to_string(i);
                    if (ImGui::TreeNode(transform_name.c_str()))
                    {
                        // Load the World matrix and decompose it into position, rotation, and scale
                        DirectX::XMVECTOR scale, rotationQuat, translation;
                        DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, DirectX::XMLoadFloat4x4(&mSkinningControllers.at(controller.first)->transforms[i]));

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

                        DirectX::XMFLOAT4X4 transform;
                        // Update the World matrix in RenderItem
                        DirectX::XMStoreFloat4x4(&transform, newWorld);
                        skinnedConstants.BoneTransforms[i] = transform;
                        mSkinningControllers[controller.first]->transforms[i] = transform;
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            currSkinnedCB->CopyData(imguiSkinningIndex, skinnedConstants);
            imguiSkinningIndex++;
        }
        ImGui::TreePop();
    }

    int blendIndex = 0;
    for (auto controller : mBlendshapeControllers)
    {
        float time = gt.TotalTime();
        BlendConstants blendConstants;

        bool animCheck = mBlendshapeControllers.at(controller.first)->animation == nullptr || mBlendshapeControllers.at(controller.first)->animation->BlendAnimNodes.size() == 0;
        if (!animCheck)
        {
            mBlendshapeControllers.at(controller.first)->UpdateBlends(gt.DeltaTime());
        }
        std::vector<Vector4> weights = GetWeights(mBlendshapeControllers.at(controller.first)->weights);
        std::copy(std::begin(weights), std::end(weights), &blendConstants.Weights[0]);
        currBlendCB->CopyData(blendIndex, blendConstants);
        blendIndex++;
    }

    if (ImGui::TreeNode("Blendshape Controllers"))
    {
        int imguiBlendIndex = 0;
        for (auto controller : mBlendshapeControllers)
        {
            BlendConstants blendConstants;
            if (ImGui::TreeNode(controller.first.c_str()))
            {
                for (auto blends : mBlendshapeControllers.at(controller.first)->weights)
                {
                    if (ImGui::TreeNode(blends.first.c_str()))
                    {
                        for (int i = 0; i < mBlendshapeControllers.at(controller.first)->weights[blends.first].size(); i++)
                        {
                            std::string blend_name = blends.first + " Blendshape" + std::to_string(i);
                            if (ImGui::TreeNode(blend_name.c_str()))
                            {
                                float weight = mBlendshapeControllers.at(controller.first)->weights[blends.first][i];
                                ImGui::InputFloat("Weight", &weight);
                                blendConstants.Weights[i].x = weight;
                                mBlendshapeControllers[controller.first]->weights[blends.first][i] = weight;
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            currBlendCB->CopyData(imguiBlendIndex, blendConstants);
            imguiBlendIndex++;
        }
        ImGui::TreePop();
    }
}