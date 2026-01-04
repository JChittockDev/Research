#pragma once

#include "../../Common/Structures.h"

class TransformNode
{
public:
	TransformNode() {};
	std::string name;
	DirectX::XMMATRIX transform;
	std::shared_ptr<TransformNode> parent;
	std::vector<std::shared_ptr<TransformNode>> children;
};