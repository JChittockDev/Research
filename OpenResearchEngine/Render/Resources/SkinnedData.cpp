#include "SkinnedData.h"

using namespace DirectX;

PositionKeyframe::PositionKeyframe() : TimePos(0.0f), Translation(0.0f, 0.0f, 0.0f)
{
}

PositionKeyframe::~PositionKeyframe()
{
}

RotateKeyframe::RotateKeyframe() : TimePos(0.0f), Rotation(0.0f, 0.0f, 0.0f, 1.0f)
{
}

RotateKeyframe::~RotateKeyframe()
{
}

ScaleKeyframe::ScaleKeyframe() : TimePos(0.0f), Scale(1.0f, 1.0f, 1.0f)
{
}

ScaleKeyframe::~ScaleKeyframe()
{
}

void aiMatConvert(const aiMatrix4x4& aiMatrix, XMFLOAT4X4& dXMatrix)
{
	XMMATRIX meshToBoneTransform = 
		XMMATRIX(aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
			aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
			aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
			aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4);

	XMStoreFloat4x4(&dXMatrix, meshToBoneTransform);
}

void aiMatConvertTranspose(const aiMatrix4x4& aiMatrix, XMFLOAT4X4& dXMatrix)
{
	XMMATRIX meshToBoneTransform = XMMatrixTranspose(
		XMMATRIX(aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
			aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
			aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
			aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4));

	XMStoreFloat4x4(&dXMatrix, meshToBoneTransform);
}
 
float BoneAnimation::GetStartTime()const
{
	std::vector<float> channelKeyStarts = { PositionKeyframes.front().TimePos, RotationKeyframes.front().TimePos , ScaleKeyframes.front().TimePos };
	
	float min = 0.0;
	for (UINT i = 0; i < channelKeyStarts.size(); ++i)
	{
		if (i == 0)
		{
			min = channelKeyStarts[i];
		}
		else if (channelKeyStarts[i] < min)
		{
			min = channelKeyStarts[i];
		}
	}
	return min;
}

float BoneAnimation::GetEndTime()const
{
	std::vector<float> channelKeyEnds = { PositionKeyframes.back().TimePos, RotationKeyframes.back().TimePos , ScaleKeyframes.back().TimePos };

	float max = 0.0;
	for (UINT i = 0; i < channelKeyEnds.size(); ++i)
	{
		if (i == 0)
		{
			max = channelKeyEnds[i];
		}
		else if (channelKeyEnds[i] > max)
		{
			max = channelKeyEnds[i];
		}
	}
	return max;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	XMVECTOR S = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Q = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR P = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	if( t <= PositionKeyframes.front().TimePos )
	{
		P = XMLoadFloat3(&PositionKeyframes.front().Translation);
	}
	else if( t >= PositionKeyframes.back().TimePos )
	{
		P = XMLoadFloat3(&PositionKeyframes.back().Translation);
	}
	else
	{
		for(UINT i = 0; i < PositionKeyframes.size()-1; ++i)
		{
			if( t >= PositionKeyframes[i].TimePos && t <= PositionKeyframes[i+1].TimePos )
			{
				float lerpPercent = (t - PositionKeyframes[i].TimePos) / (PositionKeyframes[i+1].TimePos - PositionKeyframes[i].TimePos);
				XMVECTOR p0 = XMLoadFloat3(&PositionKeyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&PositionKeyframes[i + 1].Translation);
				P = XMVectorLerp(p0, p1, lerpPercent);
				break;
			}
		}
	}

	if (t <= RotationKeyframes.front().TimePos)
	{
		Q = XMLoadFloat4(&RotationKeyframes.front().Rotation);
	}
	else if (t >= RotationKeyframes.back().TimePos)
	{
		Q = XMLoadFloat4(&RotationKeyframes.back().Rotation);
	}
	else
	{
		for (UINT i = 0; i < RotationKeyframes.size() - 1; ++i)
		{
			if (t >= RotationKeyframes[i].TimePos && t <= RotationKeyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - RotationKeyframes[i].TimePos) / (RotationKeyframes[i + 1].TimePos - RotationKeyframes[i].TimePos);
				XMVECTOR q0 = XMLoadFloat4(&RotationKeyframes[i].Rotation);
				XMVECTOR q1 = XMLoadFloat4(&RotationKeyframes[i + 1].Rotation);
				Q = XMQuaternionSlerp(q0, q1, lerpPercent);
				break;
			}
		}
	}

	if (t <= ScaleKeyframes.front().TimePos)
	{
		S = XMLoadFloat3(&ScaleKeyframes.front().Scale);
	}
	else if (t >= ScaleKeyframes.back().TimePos)
	{
		S = XMLoadFloat3(&ScaleKeyframes.back().Scale);
	}
	else
	{
		for (UINT i = 0; i < ScaleKeyframes.size() - 1; ++i)
		{
			if (t >= ScaleKeyframes[i].TimePos && t <= ScaleKeyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - ScaleKeyframes[i].TimePos) / (ScaleKeyframes[i + 1].TimePos - ScaleKeyframes[i].TimePos);
				XMVECTOR p0 = XMLoadFloat3(&ScaleKeyframes[i].Scale);
				XMVECTOR p1 = XMLoadFloat3(&ScaleKeyframes[i + 1].Scale);
				S = XMVectorLerp(p0, p1, lerpPercent);
				break;
			}
		}
	}

	XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = Math::Infinity;

	for (const auto& anim : BoneAnimations)
	{
		t = Math::Min(t, BoneAnimations.at(anim.first).GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for (const auto& anim : BoneAnimations)
	{
		t = Math::Max(t, BoneAnimations.at(anim.first).GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, const std::unordered_map<std::string, int>& boneIndex, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for (const auto& anim : BoneAnimations)
	{
		auto index = boneIndex.at(anim.first);
		BoneAnimations.at(anim.first).Interpolate(t, boneTransforms[index]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneIndex.size();
}

void SkinnedData::Set(
					  std::unordered_map<std::string, Node> boneTree,
					  std::unordered_map<std::string, int>& boneIndex,
		              std::vector<aiMatrix4x4>& boneOffsets,
		              std::unordered_map<std::string, AnimationClip>& animations,
					  std::string& rootBone,
					  aiMatrix4x4& rootMatrix)
{
	mRootMatrix    = rootMatrix;
	mBoneTree      = boneTree;
	mBoneIndex     = boneIndex;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
	mRootBone	   = rootBone;
}

void SkinnedData::TraverseToRootTransforms(const std::string& bone, const aiMatrix4x4& parentTransform, const std::vector<aiMatrix4x4>& localTransforms, const aiMatrix4x4& inverseRootMatrix, std::vector<DirectX::XMFLOAT4X4>& outputTransforms)
{
	int currentBoneIndex = mBoneIndex[bone];
	aiMatrix4x4 globalTransform = localTransforms[currentBoneIndex] * parentTransform;
	//aiMatrix4x4 globalTransform = parentTransform * localTransforms[currentBoneIndex];
	aiMatrix4x4 finalTransform = globalTransform;
	//aiMatrix4x4 finalTransform = inverseRootMatrix * globalTransform * mBoneOffsets[currentBoneIndex];
	aiMatConvert(finalTransform, outputTransforms[currentBoneIndex]);

	for (UINT i = 0; i < mBoneTree.at(bone).children.size(); ++i)
	{
		const std::string childBone = mBoneTree.at(bone).children[i];
		TraverseToRootTransforms(childBone, globalTransform, localTransforms, inverseRootMatrix, outputTransforms);
	}
}
 
void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,  std::vector<XMFLOAT4X4>& finalTransforms)
{
	UINT numBones = mBoneOffsets.size();
	std::vector<aiMatrix4x4> transforms(numBones);

	for (UINT i = 0; i < numBones; ++i)
	{
		transforms[i] = aiMatrix4x4(1.0, 0.0, 0.0, 0.0,
									0.0, 1.0, 0.0, 0.0,
									0.0, 0.0, 1.0, 0.0,
									0.0, 0.0, 0.0, 1.0);
	}

	transforms[2] = aiMatrix4x4(1.0, 0.0, 0.0, 10.0,
								 0.0, 1.0, 0.0, 0.0,
								 0.0, 0.0, 1.0, 0.0,
								 0.0, 0.0, 0.0, 1.0);

	//auto clip = mAnimations.find(clipName);
	//clip->second.Interpolate(timePos, mBoneIndex, transforms);

	TraverseToRootTransforms(mRootBone, mRootMatrix, transforms, mRootMatrix.Inverse(), finalTransforms);

}