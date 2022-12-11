#include "../EngineApp.h"


std::unique_ptr<RenderItem> buildRenderItem(std::unordered_map<std::string, std::shared_ptr<MeshRenderData>>& meshRenderAssets,
											const std::string& renderAsset, const std::string& activeMaterial, UINT& objectConstantBufferIndex, 
											UINT& skinnedConstantBufferIndex, const XMMATRIX& worldTransform, const XMMATRIX& textureTransform, 
											const std::string& activeSubmesh)
{
	std::unique_ptr<RenderItem> renderItem = std::make_unique<RenderItem>();
	renderItem->meshData = meshRenderAssets[renderAsset];

	if (renderItem->meshData->staticMesh)
	{
		renderItem->meshData->staticMesh->activeMaterial = activeMaterial;
		renderItem->meshData->staticMesh->objCBIndex = objectConstantBufferIndex++;
		XMStoreFloat4x4(&renderItem->meshData->staticMesh->worldTransform, worldTransform);
		XMStoreFloat4x4(&renderItem->meshData->staticMesh->textureTransform, textureTransform);
		renderItem->meshData->staticMesh->indexCount = renderItem->meshData->staticMesh->geometry->DrawArgs[activeSubmesh].IndexCount;
		renderItem->meshData->staticMesh->startIndexLocation = renderItem->meshData->staticMesh->geometry->DrawArgs[activeSubmesh].StartIndexLocation;
		renderItem->meshData->staticMesh->baseVertexLocation = renderItem->meshData->staticMesh->geometry->DrawArgs[activeSubmesh].BaseVertexLocation;
	}
	else if (renderItem->meshData->skinnedMesh)
	{
		renderItem->meshData->skinnedMesh->activeMaterial = activeMaterial;
		renderItem->meshData->skinnedMesh->objCBIndex = objectConstantBufferIndex++;
		renderItem->meshData->skinnedMesh->skinnedCBIndex = skinnedConstantBufferIndex++;
		XMStoreFloat4x4(&renderItem->meshData->skinnedMesh->worldTransform, worldTransform);
		XMStoreFloat4x4(&renderItem->meshData->skinnedMesh->textureTransform, textureTransform);
		renderItem->meshData->skinnedMesh->indexCount = renderItem->meshData->skinnedMesh->geometry->DrawArgs[activeSubmesh].IndexCount;
		renderItem->meshData->skinnedMesh->startIndexLocation = renderItem->meshData->skinnedMesh->geometry->DrawArgs[activeSubmesh].StartIndexLocation;
		renderItem->meshData->skinnedMesh->baseVertexLocation = renderItem->meshData->skinnedMesh->geometry->DrawArgs[activeSubmesh].BaseVertexLocation;
	}
	else
	{
		return nullptr;
	}

	return std::move(renderItem);
}

std::unique_ptr<std::vector<std::unique_ptr<RenderItem>>> buildNestedRenderItem(const std::unordered_map<std::string, std::pair<MeshType, std::string>>& inputData,
																std::unordered_map<std::string, std::shared_ptr<MeshRenderData>>& meshRenderAssets, const std::string& renderAsset, 
																UINT& objectConstantBufferIndex, UINT& skinnedConstantBufferIndex, const XMMATRIX& worldTransform, const XMMATRIX& textureTransform)
{
	std::unique_ptr<std::vector<std::unique_ptr<RenderItem>>> nestedRenderItem = std::make_unique<std::vector<std::unique_ptr<RenderItem>>>();

	size_t subMeshSize = 0;
	auto& meshRenderAsset  = meshRenderAssets[renderAsset];
	if (meshRenderAsset->staticMesh)
	{
		subMeshSize = meshRenderAsset->staticMesh->geometry->DrawArgs.size();
	}
	else if (meshRenderAsset->skinnedMesh)
	{
		subMeshSize = meshRenderAsset->skinnedMesh->geometry->DrawArgs.size();
	}
	else
	{
		return nullptr;
	}
		
	for (UINT i = 0; i < (UINT)subMeshSize; ++i)
	{
		std::string submeshName = "sm_" + std::to_string(i);
		std::string submeshMaterialName = "m_" + std::to_string(i);
		auto subMeshRenderItem = buildRenderItem(meshRenderAssets, renderAsset, submeshMaterialName, objectConstantBufferIndex, skinnedConstantBufferIndex, worldTransform, textureTransform, submeshName);
		nestedRenderItem->push_back(subMeshRenderItem);
	}
	return std::move(nestedRenderItem);
}

void sendRenderItem(std::unique_ptr<RenderItem>& renderItem, std::unordered_map<RenderLayer, std::vector<RenderItem*>>& renderItemLayers, std::vector<std::unique_ptr<RenderItem>>& renderItems, RenderLayer renderLayer)
{
	renderItemLayers[renderLayer].push_back(renderItem.get());
	renderItems.push_back(std::move(renderItem));
}

void sendNestedRenderItem(std::unique_ptr<std::vector<std::unique_ptr<RenderItem>>>& subMeshRenderItems, 
							std::unordered_map<RenderLayer, std::vector<RenderItem*>>& renderItemLayers, 
							std::vector<std::unique_ptr<RenderItem>>& renderItems, RenderLayer renderLayer)
{
	for (UINT i = 0; i < (UINT)subMeshRenderItems->size(); ++i)
	{
		sendRenderItem(subMeshRenderItems->at(i), renderItemLayers, renderItems, renderLayer);
	}
}


void EngineApp::BuildScene()
{

	// individual items example
	auto skyRitem = buildRenderItem(meshRenderAssets, "generic", "sphere", objCBIndex, skinnedCBIndex, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f), XMMatrixIdentity(), "sphere");
	auto quadRitem = buildRenderItem(meshRenderAssets, "generic", "quad", objCBIndex, skinnedCBIndex, XMMatrixIdentity(), XMMatrixIdentity(), "quad");
	auto boxRitem = buildRenderItem(meshRenderAssets, "generic", "box", objCBIndex, skinnedCBIndex, XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f), XMMatrixIdentity(), "box");
	auto gridRitem = buildRenderItem(meshRenderAssets, "generic", "grid", objCBIndex, skinnedCBIndex, XMMatrixIdentity(), XMMatrixScaling(8.0f, 8.0f, 1.0f), "grid");

	sendRenderItem(skyRitem, mRitemLayer, mAllRitems, RenderLayer::Sky);
	sendRenderItem(quadRitem, mRitemLayer, mAllRitems, RenderLayer::Debug);
	sendRenderItem(boxRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);
	sendRenderItem(gridRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);

	// instancing example
	for (int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = buildRenderItem(meshRenderAssets, "generic", "bricks0", objCBIndex, skinnedCBIndex, XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f), XMMatrixScaling(1.5f, 2.0f, 1.0f), "cylinder");
		auto rightCylRitem = buildRenderItem(meshRenderAssets, "generic", "bricks0", objCBIndex, skinnedCBIndex, XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f), XMMatrixScaling(1.5f, 2.0f, 1.0f), "cylinder");
		auto leftSphereRitem = buildRenderItem(meshRenderAssets, "generic", "mirror0", objCBIndex, skinnedCBIndex, XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f), XMMatrixScaling(1.5f, 2.0f, 1.0f), "sphere");
		auto rightSphereRitem = buildRenderItem(meshRenderAssets, "generic", "mirror0", objCBIndex, skinnedCBIndex, XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f), XMMatrixScaling(1.5f, 2.0f, 1.0f), "sphere");

		sendRenderItem(leftCylRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);
		sendRenderItem(rightCylRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);
		sendRenderItem(leftSphereRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);
		sendRenderItem(rightSphereRitem, mRitemLayer, mAllRitems, RenderLayer::Opaque);
	}

	auto skinnedRitem = buildNestedRenderItem(meshDefinitions, meshRenderAssets, "solider", objCBIndex, skinnedCBIndex,
			XMMatrixScaling(0.05f, 0.05f, -0.05f) * XMMatrixRotationY(Math::Pi) * XMMatrixTranslation(0.0f, 0.0f, -5.0f), XMMatrixIdentity());

	sendNestedRenderItem(skinnedRitem, mRitemLayer, mAllRitems, RenderLayer::SkinnedOpaque);
}