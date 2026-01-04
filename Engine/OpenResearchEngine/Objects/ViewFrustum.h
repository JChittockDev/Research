#pragma once
#include "../Common/Structures.h"
#include "../Common/Math.h"

class ViewFrustum
{
public:
	ViewFrustum() = default;
	~ViewFrustum();

	void SetNearFarTangent(float nearZ, float farZ, float tangent);

	void UpdateBounds(float aspectRatio);
	void UpdatePlanes(const DirectX::XMMATRIX& transform);
	bool IsPointInViewFrustum(const DirectX::XMVECTOR& point) const;
	bool IsSphereInViewFrustum(const DirectX::XMVECTOR& point, float radius) const;
	bool IsBoxInViewFrustum(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) const;

	float GetNear() const { return mNear; }
	float GetFar() const { return mFar; }

private:
	struct Plane
	{
		Plane() = default;
		Plane(const DirectX::XMVECTOR& p0, const DirectX::XMVECTOR& normal) : Point(p0), Normal(normal) {}

		DirectX::XMVECTOR Point = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR Normal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	};

	Plane mPlanes[6];

	float mNear = 0.1f;
	float mNearWidth = 1280.0f;
	float mNearHeight = 720.0f;

	float mFar = 1000.0f;
	float mFarWidth = 1280.0f;
	float mFarHeight = 720.0f;

	float mTangent = 0.0f;

};
