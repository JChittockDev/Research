#pragma once

#include "Joint.h"
#include "TransformNode.h"

class Skeleton
{
public:
	Skeleton() {};
	std::string rootNodeName;
	std::unordered_map<std::string, std::shared_ptr<Joint>> bones;
	void GetTransforms(float timePos, std::shared_ptr<TransformNode>& node, std::shared_ptr<Animation>& animation, DirectX::XMMATRIX& parentTransform, const DirectX::XMMATRIX& globalInverseTransform, std::vector<DirectX::XMFLOAT4X4>& transforms);
	void InitTransforms(std::shared_ptr<TransformNode>& node, DirectX::XMMATRIX& parentTransform, const DirectX::XMMATRIX& globalInverseTransform, std::vector<DirectX::XMFLOAT4X4>& transforms);
private:
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
	int FindPositionKey(float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
	int FindRotationKey(float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
	int FindScalingKey(float AnimationTime, std::unique_ptr<TransformAnimNode>& pNodeAnim);
};
