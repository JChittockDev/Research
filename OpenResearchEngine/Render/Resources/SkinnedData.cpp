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

void SkinnedData::Set(std::unordered_map<std::string, Node> boneTree,
					  std::unordered_map<std::string, int>& boneIndex,
		              std::vector<XMFLOAT4X4>& boneOffsets,
		              std::unordered_map<std::string, AnimationClip>& animations,
					  std::string& rootBone)
{
	mBoneTree      = boneTree;
	mBoneIndex     = boneIndex;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
	mRootBone	   = rootBone;
}

void SkinnedData::TraverseToRootTransforms(const std::string& bone, std::vector<XMFLOAT4X4>& transforms, std::vector<XMFLOAT4X4>& toRootTransforms)
{
	const std::string parentBone = mBoneTree.at(bone).parent;

	int currentBoneIndex = mBoneIndex[bone];
	int parentBoneIndex = mBoneIndex[parentBone];

	XMMATRIX boneTransform = XMLoadFloat4x4(&transforms[currentBoneIndex]);
	XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentBoneIndex]);
	XMMATRIX toRoot = XMMatrixMultiply(boneTransform, parentToRoot);
	XMStoreFloat4x4(&toRootTransforms[currentBoneIndex], toRoot);

	for (UINT i = 0; i < mBoneTree.at(bone).children.size(); ++i)
	{
		const std::string childBone = mBoneTree.at(bone).children[i];
		TraverseToRootTransforms(childBone, transforms, toRootTransforms);
	}

}
 
void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,  std::vector<XMFLOAT4X4>& finalTransforms)
{
	UINT numBones = mBoneOffsets.size();
	std::vector<XMFLOAT4X4> transforms(numBones);
	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	auto clip = mAnimations.find(clipName);

	clip->second.Interpolate(timePos, mBoneIndex, transforms);

	int rootIndex = mBoneIndex.at(mRootBone);
	toRootTransforms[rootIndex] = transforms[rootIndex];

	for (UINT i = 0; i < mBoneTree.at(mRootBone).children.size(); ++i)
	{
		const std::string childBone = mBoneTree.at(mRootBone).children[i];
		TraverseToRootTransforms(childBone, transforms, toRootTransforms);
	}

	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
	}

}