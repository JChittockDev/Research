#pragma once

#include "../Common/Math.h"
using namespace DirectX;

class Object
{
public:
	XMFLOAT4X4 _worldTransform = Math::Identity4x4();
};