#include "ViewFrustum.h"

ViewFrustum::~ViewFrustum()
{
}

void ViewFrustum::SetNearFarTangent(float nearZ, float farZ, float tangent)
{
	mNear = Math::Min(nearZ, farZ);
	mFar = Math::Max(nearZ, farZ);
	mTangent = tangent;
}

void ViewFrustum::UpdateBounds(float aspectRatio)
{
	mNearHeight = mNear * mTangent;
	mNearWidth = mNearHeight * aspectRatio;
	mFarHeight = mFar * mTangent;
	mFarWidth = mFarHeight * aspectRatio;
}

void ViewFrustum::UpdatePlanes(const DirectX::XMMATRIX& transform)
{
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotation;
	DirectX::XMMatrixDecompose(&scale, &rotation, &position, transform);
	DirectX::XMMATRIX invTransform = DirectX::XMMatrixInverse(nullptr, transform);
	
	DirectX::XMFLOAT4X4 invTransformTemp;
	XMStoreFloat4x4(&invTransformTemp, invTransform);
	
	const DirectX::XMVECTOR& cameraRight = DirectX::XMVectorSet(invTransformTemp.m[0][0], invTransformTemp.m[0][1], invTransformTemp.m[0][2], 0.0f);
	const DirectX::XMVECTOR& cameraUp = DirectX::XMVectorSet(invTransformTemp.m[1][0], invTransformTemp.m[1][1], invTransformTemp.m[1][2], 0.0f);
	const DirectX::XMVECTOR& cameraForward = DirectX::XMVectorSet(invTransformTemp.m[2][0], invTransformTemp.m[2][1], invTransformTemp.m[2][2], 0.0f);
	const DirectX::XMVECTOR& nearCenter = DirectX::XMVectorAdd(position, DirectX::XMVectorScale(cameraForward, mNear));
	const DirectX::XMVECTOR& farCenter = DirectX::XMVectorAdd(position, DirectX::XMVectorScale(cameraForward, mFar));
	const DirectX::XMVECTOR inverse = DirectX::XMVectorSet(-1.0f, -1.0f, -1.0f, 0.0f);

	mPlanes[4] = { nearCenter, cameraForward };
	mPlanes[5] = { farCenter, DirectX::XMVectorMultiply(cameraForward, inverse) };

	DirectX::XMVECTOR aux = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR normal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR planeVector = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	
	planeVector = DirectX::XMVectorAdd(nearCenter, DirectX::XMVectorScale(cameraUp, mNearHeight));
	aux = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(planeVector, position));
	normal = DirectX::XMVector3Cross(cameraRight, aux);
	mPlanes[0] = { planeVector, normal };

	planeVector = DirectX::XMVectorSubtract(nearCenter, DirectX::XMVectorScale(cameraUp, mNearHeight));
	aux = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(planeVector, position));
	normal = DirectX::XMVector3Cross(cameraRight, aux);
	mPlanes[1] = { planeVector, normal };

	planeVector = DirectX::XMVectorSubtract(nearCenter, DirectX::XMVectorScale(cameraRight, mNearWidth));
	aux = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(planeVector, position));
	normal = DirectX::XMVector3Cross(cameraUp, aux);
	mPlanes[2] = { planeVector, normal };

	planeVector = DirectX::XMVectorAdd(nearCenter, DirectX::XMVectorScale(cameraRight, mNearWidth));
	aux = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(planeVector, position));
	normal = DirectX::XMVector3Cross(cameraUp, aux);
	mPlanes[3] = { planeVector, normal };
}

bool ViewFrustum::IsPointInViewFrustum(const DirectX::XMVECTOR& point) const
{
	float distance = 0.0f;
	for (uint32_t i = 0; i < 6; ++i)
	{
		distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(mPlanes[i].Normal, DirectX::XMVectorSubtract(point, mPlanes[i].Point)));
		if (distance < 0.0f)
			return false;
	}

	return true;
}

bool ViewFrustum::IsSphereInViewFrustum(const DirectX::XMVECTOR& point, float radius) const
{
	bool result = true;
	float distance = 0.0f;
	for (uint32_t i = 0; i < 6; ++i)
	{
		distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(mPlanes[i].Normal, DirectX::XMVectorSubtract(point, mPlanes[i].Point)));
		if (distance < -radius)
			return false;
		else if (distance < radius)
			result = true;
	}

	return result;
}

bool ViewFrustum::IsBoxInViewFrustum(const DirectX::XMVECTOR& min, const DirectX::XMVECTOR& max) const
{
	bool result = true;

	for (uint32_t i = 0; i < 6; ++i)
	{
		const ViewFrustum::Plane& plane = mPlanes[i];

		DirectX::XMVECTOR posVert = min;
		DirectX::XMVECTOR negVert = max;

		float normalX = DirectX::XMVectorGetX(plane.Normal);
		float normalY = DirectX::XMVectorGetX(plane.Normal);
		float normalZ = DirectX::XMVectorGetX(plane.Normal);

		if (normalX >= 0)
		{
			DirectX::XMVectorSetX(posVert, DirectX::XMVectorGetX(max));
			DirectX::XMVectorSetX(negVert, DirectX::XMVectorGetX(min));
		}
		if (normalY >= 0)
		{
			DirectX::XMVectorSetY(posVert, DirectX::XMVectorGetX(max));
			DirectX::XMVectorSetY(negVert, DirectX::XMVectorGetX(min));
		}
		if (normalZ >= 0)
		{
			DirectX::XMVectorSetZ(posVert, DirectX::XMVectorGetX(max));
			DirectX::XMVectorSetZ(negVert, DirectX::XMVectorGetX(min));
		}

		if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(plane.Normal, DirectX::XMVectorSubtract(posVert, plane.Point))) < 0) 
		{ 
			return false; 
		}
		else if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(plane.Normal, DirectX::XMVectorSubtract(negVert, plane.Point))) < 0) 
		{ 
			result = true; 
		}
	}

	return result;
}
