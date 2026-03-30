#pragma once

#include "FrameResource.h"
#include "MeshInstance.h"

class RenderItem
{
public:
	RenderItem() {};

	DirectX::XMFLOAT4X4 World = Math::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Math::Identity4x4();

	int NumFramesDirty = gNumFrameResources;

	Material* Mat = nullptr;
	MeshInstance* Instance  = nullptr;  // non-owning
	std::string   SubsetName;           // key into Instance->Asset()->DrawArgs

	UINT ObjCBIndex = -1;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
