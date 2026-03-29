#include "AnimationSystem.h"
#include "../Assets/AssetManager.h"
#include "../Render/Resources/FrameResource.h"
#include "../Common/Structures.h"
#include "../Common/Math.h"
#include "../Utilities/GameTimer.h"
#include "../ImGui/imgui.h"
#include <map>
#include <vector>

static std::vector<Vector4> GetWeights(std::map<std::string, std::vector<float>>& weights)
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

void AnimationSystem::Update(const GameTimer& gt, AssetManager& assets, FrameResource& fr)
{
    ImGui::SeparatorText("Animation Controllers");

    auto currSkinnedCB = fr.SkinnedCB.get();
    auto currBlendCB   = fr.BlendCB.get();

    int skinningIndex = 0;
    std::map<std::string, SkinnedConstants> constants;
    for (auto controller : assets.mSkinningControllers)
    {
        bool animCheck = assets.mSkinningControllers.at(controller.first)->animation == nullptr ||
                         assets.mSkinningControllers.at(controller.first)->animation->TransformAnimNodes.size() == 0;
        if (!animCheck)
        {
            assets.mSkinningControllers.at(controller.first)->UpdateSkinning(gt.DeltaTime());
        }
        SkinnedConstants skinnedConstants;
        std::copy(std::begin(assets.mSkinningControllers.at(controller.first)->transforms),
                  std::end(assets.mSkinningControllers.at(controller.first)->transforms),
                  &skinnedConstants.BoneTransforms[0]);
        currSkinnedCB->CopyData(skinningIndex, skinnedConstants);
        constants[controller.first] = skinnedConstants;
        skinningIndex++;
    }

    if (ImGui::TreeNode("Skinning Controllers"))
    {
        int imguiSkinningIndex = 0;
        for (auto controller : assets.mSkinningControllers)
        {
            SkinnedConstants skinnedConstants = constants[controller.first];
            if (ImGui::TreeNode(controller.first.c_str()))
            {
                for (int i = 0; i < assets.mSkinningControllers.at(controller.first)->transforms.size(); i++)
                {
                    std::string transform_name = "Transform " + std::to_string(i);
                    if (ImGui::TreeNode(transform_name.c_str()))
                    {
                        DirectX::XMVECTOR scale, rotationQuat, translation;
                        DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation,
                            DirectX::XMLoadFloat4x4(&assets.mSkinningControllers.at(controller.first)->transforms[i]));

                        DirectX::XMFLOAT3 position, rotationEuler, scaleValues;
                        DirectX::XMStoreFloat3(&position, translation);
                        DirectX::XMStoreFloat3(&scaleValues, scale);

                        DirectX::XMFLOAT4 rotationQuatValues;
                        DirectX::XMStoreFloat4(&rotationQuatValues, rotationQuat);
                        rotationEuler = Math::QuaternionToEuler(rotationQuatValues);

                        ImGui::DragFloat3("Position", &position.x);
                        ImGui::DragFloat3("Rotation (Euler)", &rotationEuler.x);
                        ImGui::DragFloat3("Scale", &scaleValues.x);

                        DirectX::XMVECTOR newScale       = DirectX::XMLoadFloat3(&scaleValues);
                        DirectX::XMVECTOR newRotation    = Math::EulerToQuaternion(rotationEuler);
                        DirectX::XMVECTOR newTranslation = DirectX::XMLoadFloat3(&position);

                        DirectX::XMMATRIX newWorld =
                            DirectX::XMMatrixScalingFromVector(newScale) *
                            DirectX::XMMatrixRotationQuaternion(newRotation) *
                            DirectX::XMMatrixTranslationFromVector(newTranslation);

                        DirectX::XMFLOAT4X4 transform;
                        DirectX::XMStoreFloat4x4(&transform, newWorld);
                        skinnedConstants.BoneTransforms[i]                              = transform;
                        assets.mSkinningControllers[controller.first]->transforms[i]   = transform;
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
    for (auto controller : assets.mBlendshapeControllers)
    {
        BlendConstants blendConstants;

        bool animCheck = assets.mBlendshapeControllers.at(controller.first)->animation == nullptr ||
                         assets.mBlendshapeControllers.at(controller.first)->animation->BlendAnimNodes.size() == 0;
        if (!animCheck)
        {
            assets.mBlendshapeControllers.at(controller.first)->UpdateBlends(gt.DeltaTime());
        }
        std::vector<Vector4> weights = GetWeights(assets.mBlendshapeControllers.at(controller.first)->weights);
        std::copy(std::begin(weights), std::end(weights), &blendConstants.Weights[0]);
        currBlendCB->CopyData(blendIndex, blendConstants);
        blendIndex++;
    }

    if (ImGui::TreeNode("Blendshape Controllers"))
    {
        int imguiBlendIndex = 0;
        for (auto controller : assets.mBlendshapeControllers)
        {
            BlendConstants blendConstants;
            if (ImGui::TreeNode(controller.first.c_str()))
            {
                for (auto blends : assets.mBlendshapeControllers.at(controller.first)->weights)
                {
                    if (ImGui::TreeNode(blends.first.c_str()))
                    {
                        for (int i = 0; i < assets.mBlendshapeControllers.at(controller.first)->weights[blends.first].size(); i++)
                        {
                            std::string blend_name = blends.first + " Blendshape" + std::to_string(i);
                            if (ImGui::TreeNode(blend_name.c_str()))
                            {
                                float weight = assets.mBlendshapeControllers.at(controller.first)->weights[blends.first][i];
                                ImGui::InputFloat("Weight", &weight);
                                blendConstants.Weights[i].x                                                     = weight;
                                assets.mBlendshapeControllers[controller.first]->weights[blends.first][i] = weight;
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
