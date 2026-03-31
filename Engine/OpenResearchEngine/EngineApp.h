#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Assets/AssetManager.h"
#include "Animation/AnimationSystem.h"
#include "Render/RenderPipeline.h"
#include "Common/SceneState.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/ShadowMap.h"
#include "Render/Resources/Ssao.h"
#include "Render/Resources/SSS.h"
#include "Render/Resources/Composite.h"
#include "Render/Resources/Lighting.h"
#include "Render/Resources/Ssgi.h"
#include "Render/Resources/ShadowResources.h"
#include "Render/Resources/RadianceResources.h"
#include "Render/Resources/GBuffer.h"
#include "Render/Resources/RenderTextures.h"

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

    void SetRenderPassResources();
    void BuildScene();

    void Render(FrameResource* currentFrameResource);

private:
    POINT      mLastMousePos;
    Camera     mCamera;
    SceneState mSceneState;

    std::unique_ptr<AssetManager>    mAssets;
    std::unique_ptr<AnimationSystem> mAnimationSystem;
    std::unique_ptr<RenderPipeline>  mRenderPipeline;

    int            mCurrFrameResourceIndex = 0;
    FrameResource* mCurrFrameResource      = nullptr;
    std::vector<std::shared_ptr<FrameResource>> mFrameResources;

    std::unique_ptr<GBuffer>           mGBuffer;
    std::unique_ptr<Lighting>          mLighting;
    std::unique_ptr<Ssao>              mSsao;
    std::unique_ptr<Ssgi>              mSsgi;
    std::unique_ptr<SSS>               mSss;
    std::unique_ptr<Composite>         mComposite;
    std::unique_ptr<RenderTextures>    mRenderTextures;
    std::unique_ptr<ShadowResources>   mShadowResources;
    std::unique_ptr<RadianceResources> mRadianceResources;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap;
};
