#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "../../Common/Structures.h"
#include "../../D3D12/D3DUtil.h"
#include "../../Common/Math.h"

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct PositionKeyframe
{
	PositionKeyframe();
	~PositionKeyframe();

    float TimePos;
	DirectX::XMFLOAT3 Translation;
};

struct RotateKeyframe
{
	RotateKeyframe();
	~RotateKeyframe();

	float TimePos;
	DirectX::XMFLOAT4 Rotation;
};

struct ScaleKeyframe
{
	ScaleKeyframe();
	~ScaleKeyframe();

	float TimePos;
	DirectX::XMFLOAT3 Scale;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;

	std::vector<PositionKeyframe> PositionKeyframes;
	std::vector<RotateKeyframe> RotationKeyframes;
	std::vector<ScaleKeyframe> ScaleKeyframes;
};

///<summary>
/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
/// An AnimationClip requires a BoneAnimation for every bone to form
/// the animation clip.    
///</summary>
struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

    void Interpolate(float t, const std::unordered_map<std::string, int>& boneIndex, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

	std::unordered_map<std::string, BoneAnimation> BoneAnimations;
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::unordered_map<std::string, Node> boneTree,
		std::unordered_map<std::string, int>& boneIndex,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations,
		std::string& rootBone);

	 // In a real project, you'd want to cache the result if there was a chance
	 // that you were calling this several times with the same clipName at 
	 // the same timePos.
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<DirectX::XMFLOAT4X4>& finalTransforms);

	void TraverseToRootTransforms(const std::string& bone, std::vector<DirectX::XMFLOAT4X4>& transforms,
		std::vector<DirectX::XMFLOAT4X4>& toRootTransforms);

private:
	std::string mRootBone;
	std::unordered_map<std::string, Node> mBoneTree;
	std::unordered_map<std::string, int> mBoneIndex;
	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;
	std::unordered_map<std::string, AnimationClip> mAnimations;
};
 
#endif // SKINNEDDATA_H