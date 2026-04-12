#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "../Common/Structures.h"

class IRenderPassResourceArray
{
public:
	virtual UINT Width() = 0;
	virtual UINT Height() = 0;
	virtual UINT Divisor() = 0;

	virtual void OnResize(UINT newWidth, UINT newHeight, UINT divisor) = 0;
};
