//***************************************************************************************
// Math.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

class Math
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

    static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius*sinf(phi)*cosf(theta),
			radius*cosf(phi),
			radius*sinf(phi)*sinf(theta),
			1.0f);
	}

    static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
        DirectX::XMMATRIX A = M;
        A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

    static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

    static DirectX::XMVECTOR RandUnitVec3();
    static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;

	static DirectX::XMFLOAT4X4 CreateTransformMatrix(const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& scaling)
	{
		DirectX::XMFLOAT4X4 matrix;
		DirectX::XMVECTOR translationVector = DirectX::XMLoadFloat3(&translation);
		DirectX::XMVECTOR rotationQuaternion = DirectX::XMLoadFloat4(&rotation);
		DirectX::XMVECTOR scalingVector = DirectX::XMLoadFloat3(&scaling);
		DirectX::XMMATRIX scalingMatrix = DirectX::XMMatrixScalingFromVector(scalingVector);
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(rotationQuaternion);
		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(translationVector);
		DirectX::XMMATRIX transformMatrix = scalingMatrix * rotationMatrix * translationMatrix;
		DirectX::XMStoreFloat4x4(&matrix, transformMatrix);
		return matrix;
	}

	static DirectX::XMVECTOR Difference(const DirectX::XMFLOAT3& point1, const DirectX::XMFLOAT3& point2)
	{
		DirectX::XMVECTOR vec1 = DirectX::XMLoadFloat3(&point1);
		DirectX::XMVECTOR vec2 = DirectX::XMLoadFloat3(&point2);
		DirectX::XMVECTOR diffVec = DirectX::XMVectorSubtract(vec2, vec1);

		return diffVec;
	}

	static float Length(const DirectX::XMFLOAT3& point1, const DirectX::XMFLOAT3& point2)
	{
		DirectX::XMVECTOR vec1 = DirectX::XMLoadFloat3(&point1);
		DirectX::XMVECTOR vec2 = DirectX::XMLoadFloat3(&point2);
		DirectX::XMVECTOR diffVec = Difference(point1, point2);
		float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(diffVec));

		return length;
	}

	static DirectX::XMFLOAT3 QuaternionToEuler(const DirectX::XMFLOAT4& quat);

	static DirectX::XMVECTOR EulerToQuaternion(const DirectX::XMFLOAT3& euler);
};

