#include "Skeleton.h"

int Skeleton::FindPositionKey(float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->positionKeys.size() - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->positionKeys[i + 1]->mTime) {
            return i;
        }
    }
    return 0;
}


int Skeleton::FindRotationKey(float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->rotationKeys.size() - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->rotationKeys[i + 1]->mTime) {
            return i;
        }
    }
    return 0;
}

int Skeleton::FindScalingKey(float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->scalingKeys.size() - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->scalingKeys[i + 1]->mTime) {
            return i;
        }
    }
    return 0;
}

void Skeleton::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    if (pNodeAnim->positionKeys.size() == 1) {
        Out = pNodeAnim->positionKeys[0]->mValue;
        return;
    }

    int PositionIndex = FindPositionKey(AnimationTime, pNodeAnim);
    int NextPositionIndex = (PositionIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->positionKeys[NextPositionIndex]->mTime - pNodeAnim->positionKeys[PositionIndex]->mTime);
    float Factor = Math::Clamp((AnimationTime - (float)pNodeAnim->positionKeys[PositionIndex]->mTime) / DeltaTime, 0.0f, 1.0f);
   
    const aiVector3D& Start = pNodeAnim->positionKeys[PositionIndex]->mValue;
    const aiVector3D& End = pNodeAnim->positionKeys[NextPositionIndex]->mValue;
    aiVector3D Delta = End - Start;
    
    Out = Start + Factor * Delta;
}

void Skeleton::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    if (pNodeAnim->rotationKeys.size() == 1) {
        Out = pNodeAnim->rotationKeys[0]->mValue;
        return;
    }

    int RotationIndex = FindRotationKey(AnimationTime, pNodeAnim);
    int NextRotationIndex = (RotationIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->rotationKeys[NextRotationIndex]->mTime - pNodeAnim->rotationKeys[RotationIndex]->mTime);
    float Factor = Math::Clamp((AnimationTime - (float)pNodeAnim->rotationKeys[RotationIndex]->mTime) / DeltaTime, 0.0f, 1.0f);
    
    const aiQuaternion& StartRotationQ = pNodeAnim->rotationKeys[RotationIndex]->mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->rotationKeys[NextRotationIndex]->mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    
    Out = Out.Normalize();
}

void Skeleton::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, std::unique_ptr<AnimationNode>& pNodeAnim)
{
    if (pNodeAnim->scalingKeys.size() == 1) {
        Out = pNodeAnim->scalingKeys[0]->mValue;
        return;
    }

    int ScalingIndex = FindScalingKey(AnimationTime, pNodeAnim);
    int NextScalingIndex = (ScalingIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->scalingKeys[NextScalingIndex]->mTime - pNodeAnim->scalingKeys[ScalingIndex]->mTime);
    float Factor = Math::Clamp((AnimationTime - (float)pNodeAnim->scalingKeys[ScalingIndex]->mTime) / DeltaTime, 0.0f, 1.0f);
    
    const aiVector3D& Start = pNodeAnim->scalingKeys[ScalingIndex]->mValue;
    const aiVector3D& End = pNodeAnim->scalingKeys[NextScalingIndex]->mValue;
    aiVector3D Delta = End - Start;
    
    Out = Start + Factor * Delta;
}

DirectX::XMMATRIX CreateAffineMatrix(const aiVector3D& scaling, const aiQuaternion& rotation, const aiVector3D& translate)
{
    DirectX::XMMATRIX scalingMatrix = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);
    DirectX::XMMATRIX affineMatrix = scalingMatrix * rotationMatrix * translationMatrix;
    return affineMatrix;
}

void Skeleton::GetTransforms(float timePos, std::shared_ptr<TransformNode>& node, std::shared_ptr<Animation>& animation, DirectX::XMMATRIX& parentTransform, const DirectX::XMMATRIX& globalInverseTransform, std::vector<DirectX::XMFLOAT4X4>& transforms)
{
    DirectX::XMMATRIX nodeTransform = node->transform;

    if (animation->animationNodes.find(node->name) != animation->animationNodes.end())
    {
        aiVector3D scaling;
        aiQuaternion rotation;
        aiVector3D translate;
        CalcInterpolatedScaling(scaling, timePos, animation->animationNodes[node->name]);
        CalcInterpolatedRotation(rotation, timePos, animation->animationNodes[node->name]);
        CalcInterpolatedPosition(translate, timePos, animation->animationNodes[node->name]);
        nodeTransform = DirectX::XMMatrixTranspose(CreateAffineMatrix(scaling, rotation, translate));
    }

    DirectX::XMMATRIX globalTransform = parentTransform * nodeTransform;

    if (this->bones.find(node->name) != this->bones.end())
    {
        int boneIndex = this->bones[node->name]->index;
        DirectX::XMMATRIX finalTransform = globalInverseTransform * globalTransform * this->bones[node->name]->inverseBindMatrix;
        DirectX::XMStoreFloat4x4(&transforms[boneIndex], finalTransform);
    }

    for (UINT i = 0u; i < node->children.size(); i++)
    {
        GetTransforms(timePos, node->children[i], animation, globalTransform, globalInverseTransform, transforms);
    }
}