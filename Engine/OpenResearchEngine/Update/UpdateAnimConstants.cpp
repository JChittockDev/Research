#include "../EngineApp.h"

void EngineApp::UpdateAnimCBs(const GameTimer& gt)
{
    ImGui::SeparatorText("Animation Controllers");

    auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();
    auto currBlendCB   = mCurrFrameResource->BlendCB.get();

    // ── Update skinning controllers ──
    for (SkinDeformer* sd : mSkinDeformers) {
        SkinningController* ctrl = sd->Controller();
        if (!ctrl) continue;
        bool hasAnim = ctrl->animation &&
                       !ctrl->animation->TransformAnimNodes.empty();
        if (hasAnim)
            ctrl->UpdateSkinning(gt.DeltaTime());

        SkinnedConstants skc;
        std::copy(ctrl->transforms.begin(), ctrl->transforms.end(),
                  &skc.BoneTransforms[0]);
        currSkinnedCB->CopyData(sd->CBIndex(), skc);
    }

    if (ImGui::TreeNode("Skinning Controllers")) {
        for (SkinDeformer* sd : mSkinDeformers) {
            SkinningController* ctrl = sd->Controller();
            if (!ctrl) continue;
            // No name field on SkinningController — use fallback label
            const char* label = "Controller";
            if (ImGui::TreeNode(label)) {
                for (int i = 0; i < (int)ctrl->transforms.size(); i++) {
                    std::string tname = "Transform " + std::to_string(i);
                    if (ImGui::TreeNode(tname.c_str())) {
                        DirectX::XMVECTOR scale, rot, trans;
                        DirectX::XMMatrixDecompose(&scale, &rot, &trans,
                            DirectX::XMLoadFloat4x4(&ctrl->transforms[i]));
                        DirectX::XMFLOAT3 pos, sc;
                        DirectX::XMFLOAT4 rq;
                        DirectX::XMStoreFloat3(&pos, trans);
                        DirectX::XMStoreFloat3(&sc, scale);
                        DirectX::XMStoreFloat4(&rq, rot);
                        DirectX::XMFLOAT3 euler = Math::QuaternionToEuler(rq);
                        ImGui::DragFloat3("Position", &pos.x);
                        ImGui::DragFloat3("Rotation", &euler.x);
                        ImGui::DragFloat3("Scale",    &sc.x);
                        DirectX::XMMATRIX newW =
                            DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sc))
                            * DirectX::XMMatrixRotationQuaternion(Math::EulerToQuaternion(euler))
                            * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
                        DirectX::XMStoreFloat4x4(&ctrl->transforms[i], newW);
                        SkinnedConstants skc;
                        std::copy(ctrl->transforms.begin(), ctrl->transforms.end(),
                                  &skc.BoneTransforms[0]);
                        currSkinnedCB->CopyData(sd->CBIndex(), skc);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    // ── Update blendshape controllers ──
    for (BlendshapeDeformer* bd : mBlendshapeDeformers) {
        BlendshapeController* ctrl = bd->Controller();
        if (!ctrl) continue;
        bool hasAnim = ctrl->animation &&
                       !ctrl->animation->BlendAnimNodes.empty();
        if (hasAnim)
            ctrl->UpdateBlends(gt.DeltaTime());

        BlendConstants bc;
        int slot = 0;
        for (auto& [setName, wts] : ctrl->weights) {
            for (float w : wts) {
                if (slot < 64) bc.Weights[slot++].x = w;
            }
        }
        currBlendCB->CopyData(bd->CBIndex(), bc);
    }

    if (ImGui::TreeNode("Blendshape Controllers")) {
        for (BlendshapeDeformer* bd : mBlendshapeDeformers) {
            BlendshapeController* ctrl = bd->Controller();
            if (!ctrl) continue;
            // No name field on BlendshapeController — use fallback label
            const char* label = "Controller";
            if (ImGui::TreeNode(label)) {
                for (auto& [setName, wts] : ctrl->weights) {
                    if (ImGui::TreeNode(setName.c_str())) {
                        BlendConstants bc;
                        for (int i = 0; i < (int)wts.size(); i++) {
                            std::string bn = setName + " Blendshape" + std::to_string(i);
                            if (ImGui::TreeNode(bn.c_str())) {
                                ImGui::InputFloat("Weight", &wts[i]);
                                bc.Weights[i].x = wts[i];
                                ImGui::TreePop();
                            }
                        }
                        currBlendCB->CopyData(bd->CBIndex(), bc);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}