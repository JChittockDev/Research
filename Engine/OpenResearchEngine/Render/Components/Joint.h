#pragma once

#include "../../Common/Structures.h"

class Joint
{
public:
	Joint() {};

	Joint(std::string jointName, int jointIndex, DirectX::XMMATRIX jointInverseBindMatrix)
	{
		name = jointName;
		index = jointIndex;
		inverseBindMatrix = jointInverseBindMatrix;
	};

	std::string name;
	int index;
	DirectX::XMMATRIX inverseBindMatrix;
};
