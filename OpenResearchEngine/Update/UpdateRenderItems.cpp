#include "../EngineApp.h"

void EngineApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mMeshRenderItemMap)
	{
		std::vector<std::shared_ptr<RenderItem>>& renderItems = e.second;
		for (int i = 0; i < renderItems.size(); i++)
		{
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (renderItems[i]->NumFramesDirty > 0)
			{
				DirectX::XMMATRIX world = XMLoadFloat4x4(&renderItems[i]->World);
				DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&renderItems[i]->TexTransform);

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
				objConstants.MaterialIndex = renderItems[i]->Mat->MatCBIndex;

				currObjectCB->CopyData(renderItems[i]->ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				renderItems[i]->NumFramesDirty--;
			}
		}
	}
}