#pragma once

#include "../../Render/Resources/Joint.h"
#include "assimp/scene.h"
#include <math.h>

class Skeleton
{
public:
	std::unordered_map<std::string, Joint> bones;
	void GetTransforms(float timePos, aiNode* node, aiAnimation* animation, aiMatrix4x4& parentTransform, const aiMatrix4x4& globalInverseTransform, std::vector<DirectX::XMFLOAT4X4>& transforms);

private:
	const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
	int FindPositionKey(float AnimationTime, const aiNodeAnim* pNodeAnim);
	int FindRotationKey(float AnimationTime, const aiNodeAnim* pNodeAnim);
	int FindScalingKey(float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
};
