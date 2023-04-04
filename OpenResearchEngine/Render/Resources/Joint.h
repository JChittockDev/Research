#pragma once

#include "../../Common/Structures.h"
#include "assimp/scene.h"

class Joint
{
public:
	Joint() {};

	Joint(std::string jointName, int jointIndex, aiMatrix4x4 jointOffsetMatrix)
	{
		name = jointName;
		index = jointIndex;
		offsetMatrix = jointOffsetMatrix;
	};

	std::string name;
	int index;
	aiMatrix4x4 offsetMatrix;
};
