#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/ShadowPassResource.h"
#include "Render/Resources/RadiancePassResource.h"
#include "Render/Resources/SsaoPassResource.h"
#include "Render/Resources/SssPassResource.h"
#include "Render/Resources/CompositePassResource.h"
#include "Render/Resources/LightingPassResource.h"
#include "Render/Resources/SsgiPassResource.h"
#include "Render/Resources/GBufferPassResource.h"
#include "Render/RenderItem.h"
#include "Render/RenderPassResourceArray.h"
#include "Render/Resources/RenderTextures.h"
#include "Render/RenderPipeline.h"
#include "Assets/AssetManager.h"
#include "Utilities/OnnxModelResource.h"
#include "Serialize/LevelReader.h"

using Microsoft::WRL::ComPtr;

class EngineApp : public D3DApp
{
public:
    EngineApp(HINSTANCE hInstance);
    EngineApp(const EngineApp& rhs) = delete;
    EngineApp& operator=(const EngineApp& rhs) = delete;
    ~EngineApp();

    virtual bool Initialize() override;

private:
    virtual void CreateRtvAndDsvDescriptorHeaps() override;
    virtual void OnResize() override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt) override;
    virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
    void OnKeyboardInput(const GameTimer& gt);

    void UpdateRenderAssets(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateAnimCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
    void UpdateLights(const GameTimer& gt);
    void UpdateLightTransforms(const std::vector<LightTransform>& lights, DirectX::XMFLOAT4X4* LightTransforms);
    void UpdateScreenSpaceCB(const GameTimer& gt);
    void UpdateSssCB(const GameTimer& gt);
    void UpdateRadiancePassCB(const GameTimer& gt);

    void PushLights();
    void PushRenderItems();
    void SetFenceResources();
    void SetRenderPassResources();
    void SerializeLevel();
    void BuildScene();
    void BuildPipeline();

    void Render(FrameResource* currentFrameResource);

    void SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms);
    std::string extractFileName(const std::string& filePath);

private:
    POINT mLastMousePos;
    Camera mCamera;

    DirectX::BoundingSphere mSceneBounds;

    DynamicLights dynamicLights;

    std::vector<std::shared_ptr<RenderItem>> mRenderItems;
    std::vector<std::shared_ptr<FrameResource>> mFrameResources;
    int mCurrFrameResourceIndex = 0;
    FrameResource* mCurrFrameResource = nullptr;

    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mMeshRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mDeformedRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mDirectionalLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mSpotLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mRenderItemLayers;

    std::unordered_map<std::string, std::unordered_map<std::string, ItemData>>        mLevelRenderItems;
    std::unordered_map<std::string, std::unordered_map<std::string, PBRMaterialData>> mLevelMaterials;
    std::unordered_map<std::string, std::unordered_map<std::string, LightData>>       mLevelLights;

    UINT BlendCBIndex    = 0;
    UINT SkinnedCBIndex  = 0;
    UINT ObjectCBIndex   = 0;

    PassConstants                mMainPassCB;
    std::vector<PassConstants>   mShadowPassCBs;
    std::vector<RadianceConstants> mRadianceCBs;

    std::unique_ptr<OnnxModelResource> mOnnxModelResource;

    std::unique_ptr<GBufferPassResource>          mGBuffer;
    std::unique_ptr<LightingPassResource> mLighting;
    std::unique_ptr<SsaoPassResource>             mSsao;
    std::unique_ptr<SsgiPassResource>             mSsgi;
    std::unique_ptr<SssPassResource>              mSss;
    std::unique_ptr<CompositePassResource>        mComposite;
    std::unique_ptr<RenderTextures>   mRenderTextures;

    std::unique_ptr<RenderPassResourceArray> mShadowResourceArray;
    std::unique_ptr<RenderPassResourceArray> mRadianceResourceArray;

    std::unique_ptr<AssetManager> mAssets;
    RenderPipeline mPipeline;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap  = nullptr;
    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap = nullptr;
};
