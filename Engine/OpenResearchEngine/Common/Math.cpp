//***************************************************************************************
#include "Math.h"
#include <cmath>
#include <vector>
#include <random>

using namespace DirectX;

const float Math::Infinity = FLT_MAX;
const float Math::Pi       = 3.1415926535f;

void Math::GenerateRandomVectors(const UINT& width, const UINT& height, std::vector<DirectX::XMFLOAT4>& vectors)
{
	vectors.resize(width * height);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	for (UINT i = 0; i < height; ++i)
	{
		for (UINT j = 0; j < width; ++j)
		{
			UINT index = i * width + j;

			vectors[index] = XMFLOAT4(
				dis(gen), // R: for radius sampling
				dis(gen), // G: for disk sampling X
				dis(gen), // B: for disk sampling Y  
				dis(gen)  // A: extra random value for future use
			);
		}
	}
}

void Math::GenerateBoxVectors(DirectX::XMFLOAT4 OffsetVectors[14])
{
	OffsetVectors[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	OffsetVectors[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
	OffsetVectors[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	OffsetVectors[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);
	OffsetVectors[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	OffsetVectors[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);
	OffsetVectors[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	OffsetVectors[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);
	OffsetVectors[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	OffsetVectors[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);
	OffsetVectors[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	OffsetVectors[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);
	OffsetVectors[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	OffsetVectors[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		float s = Math::RandF(0.25f, 1.0f);
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&OffsetVectors[i]));
		XMStoreFloat4(&OffsetVectors[i], v);
	}
}

std::vector<float> Math::GenerateGaussWeights(float sigma, UINT MaxBlurRadius)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

DirectX::XMFLOAT3 Math::QuaternionToEuler(const DirectX::XMFLOAT4& quat)
{
	DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&quat);
	DirectX::XMFLOAT3 euler;
	euler.x = atan2(2.0f * (quat.w * quat.x + quat.y * quat.z), 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y));
	euler.y = asin(2.0f * (quat.w * quat.y - quat.z * quat.x));
	euler.z = atan2(2.0f * (quat.w * quat.z + quat.x * quat.y), 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z));
	return euler;
}

DirectX::XMVECTOR Math::EulerToQuaternion(const DirectX::XMFLOAT3& euler)
{
	using namespace DirectX;
	return XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
}

float Math::AngleFromXY(float x, float y)
{
	float theta = 0.0f;
 
	// Quadrant I or IV
	if(x >= 0.0f) 
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if(theta < 0.0f)
			theta += 2.0f*Pi; // in [0, 2*pi).
	}

	// Quadrant II or III
	else      
		theta = atanf(y/x) + Pi; // in [0, 2*pi).

	return theta;
}

XMVECTOR Math::RandUnitVec3()
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.

		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR Math::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f), Math::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		
		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		// Ignore points in the bottom hemisphere.
		if( XMVector3Less( XMVector3Dot(n, v), Zero ) )
			continue;

		return XMVector3Normalize(v);
	}
}