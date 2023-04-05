#include "Skeleton.h"

float clamp(const float& f)
{
    return (f < 0.0f) ? 0.0f : ((f > 1.0f) ? 1.0f : f);
}

void aiMatConvert(const aiMatrix4x4& aiMatrix, DirectX::XMFLOAT4X4& dXMatrix)
{
	DirectX::XMMATRIX meshToBoneTransform = DirectX::XMMATRIX(aiMatrix.a1, aiMatrix.a2, 
                                            aiMatrix.a3, aiMatrix.a4, aiMatrix.b1, aiMatrix.b2, 
                                            aiMatrix.b3, aiMatrix.b4, aiMatrix.c1, aiMatrix.c2, 
                                            aiMatrix.c3, aiMatrix.c4, aiMatrix.d1, aiMatrix.d2, 
                                            aiMatrix.d3, aiMatrix.d4);

	DirectX::XMStoreFloat4x4(&dXMatrix, meshToBoneTransform);
}

const aiNodeAnim* Skeleton::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
{
    for (unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.data) == nodeName)
        {
            return nodeAnim;
        }
    }

    return nullptr;
}

int Skeleton::FindPositionKey(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }
    return 0;
}


int Skeleton::FindRotationKey(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }
    return 0;
}

int Skeleton::FindScalingKey(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }
    return 0;
}

void Skeleton::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    int PositionIndex = FindPositionKey(AnimationTime, pNodeAnim);
    int NextPositionIndex = (PositionIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = clamp((AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime);
   
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    
    Out = Start + Factor * Delta;
}

void Skeleton::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    int RotationIndex = FindRotationKey(AnimationTime, pNodeAnim);
    int NextRotationIndex = (RotationIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor = clamp((AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime);
    
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    
    Out = Out.Normalize();
}

void Skeleton::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    int ScalingIndex = FindScalingKey(AnimationTime, pNodeAnim);
    int NextScalingIndex = (ScalingIndex + 1);
    
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor = clamp((AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime);
    
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    
    Out = Start + Factor * Delta;
}

aiMatrix4x4 CreateAffineMatrix(const aiVector3D& scaling, const aiQuaternion& rotation, const aiVector3D& translate)
{
    aiMatrix4x4 scalingMatrix;
    aiMatrix4x4 translationMatrix;
    aiMatrix4x4::Scaling(scaling, scalingMatrix);
    aiMatrix4x4::Translation(translate, translationMatrix);
    aiMatrix4x4 rotationMatrix = aiMatrix4x4(rotation.GetMatrix());

    return scalingMatrix * rotationMatrix * translationMatrix;
}

void Skeleton::GetTransforms(float timePos, aiNode* node, aiAnimation* animation, aiMatrix4x4& parentTransform, const aiMatrix4x4& globalInverseTransform, std::vector<DirectX::XMFLOAT4X4>& transforms)
{
	std::string nodeName(node->mName.data);
	aiMatrix4x4 nodeTransform = node->mTransformation;
    const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);

    if (nodeAnim)
    {
        aiVector3D scaling;
        aiQuaternion rotation;
        aiVector3D translate;

        CalcInterpolatedScaling(scaling, timePos, nodeAnim);
        CalcInterpolatedRotation(rotation, timePos, nodeAnim);
        CalcInterpolatedPosition(translate, timePos, nodeAnim);

        nodeTransform = CreateAffineMatrix(scaling, rotation, translate);
    }

    aiMatrix4x4 globalTransform = parentTransform * nodeTransform;

	if (this->bones.find(nodeName) != this->bones.end())
	{
		int boneIndex = this->bones[nodeName].index;
		aiMatrix4x4 finalTransform = globalInverseTransform * globalTransform * this->bones[nodeName].offsetMatrix;
		aiMatConvert(finalTransform, transforms[boneIndex]);
	}

	for (UINT i = 0u; i < node->mNumChildren; i++)
	{
		GetTransforms(timePos, node->mChildren[i], animation, globalTransform, globalInverseTransform, transforms);
	}
}