#include "../EngineApp.h"


void EngineApp::BuildRenderItems()
{
	RenderItem::BuildRenderItem("shapeGeo", "sphere", "sky", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0), 
		DirectX::XMFLOAT3(5000.0f, 5000.0f, 5000.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Sky], mAllRitems);

	RenderItem::BuildRenderItem("shapeGeo", "quad", "bricks0", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Debug], mAllRitems);

	RenderItem::BuildRenderItem("shapeGeo", "box", "bricks0", DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
		DirectX::XMFLOAT3(2.0f, 1.0f, 2.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);

	RenderItem::BuildRenderItem("shapeGeo", "grid", "tile0", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);

	for (int i = 0; i < 5; ++i)
	{
		RenderItem::BuildRenderItem("shapeGeo", "cylinder", "bricks0", DirectX::XMFLOAT3(-5.0f, 1.5f, -10.0f + i * 5.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
			DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);

		RenderItem::BuildRenderItem("shapeGeo", "cylinder", "bricks0", DirectX::XMFLOAT3(5.0f, 1.5f, -10.0f + i * 5.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
			DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);

		RenderItem::BuildRenderItem("shapeGeo", "sphere", "mirror0", DirectX::XMFLOAT3(-5.0f, 3.5f, -10.0f + i * 5.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
			DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);

		RenderItem::BuildRenderItem("shapeGeo", "sphere", "mirror0", DirectX::XMFLOAT3(5.0f, 3.5f, -10.0f + i * 5.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0),
			DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), ObjectCBIndex, mGeometries, mMaterials, mRitemLayer[(int)RenderLayer::Opaque], mAllRitems);
	}

	RenderItem::BuildRenderItems("Models\\test.fbx", DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f), DirectX::XMFLOAT4(0.0, 0.0, 0.0, 1.0), DirectX::XMFLOAT3(0.05f, 0.05f, -0.05f),
								ObjectCBIndex, SkinnedCBIndex, mSkinnedSubsets, mGeometries, mMaterials, mSkinnedMats, mSkinnedMesh, mRitemLayer[(int)RenderLayer::SkinnedOpaque], mAllRitems);
}