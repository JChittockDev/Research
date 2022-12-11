#pragma once

#include "FrameResource.h"
#include "../../Objects/MeshRenderData.h"

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;
	RenderItem(const RenderItem& rhs) = delete;
	
	std::shared_ptr<MeshRenderData> meshData;
	
	int NumFramesDirty = gNumFrameResources;
};