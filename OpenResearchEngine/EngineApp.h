#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Models/Internal/GeometryGenerator.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/Mesh.h"
#include "Shaders/Compiler/ShaderCompiler.h"
#include "Render/Passes/ShadowMap.h"
#include "Render/Passes/Ssao.h"
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/Animation.h"
#include "Serialize/LevelReader.h"

using Microsoft::WRL::ComPtr;

class EngineApp : public D3DApp
{
public:
    EngineApp(HINSTANCE hInstance);
    EngineApp(const EngineApp& rhs) = delete;
    EngineApp& operator=(const EngineApp& rhs) = delete;
    ~EngineApp();

    virtual bool Initialize()override;

private:
    virtual void CreateRtvAndDsvDescriptorHeaps()override;
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
    void OnKeyboardInput(const GameTimer& gt);
    
    void UpdateRenderAssets(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateSkinnedCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
    void UpdateSsaoCB(const GameTimer& gt);
    void UpdateLights(const GameTimer& gt);
    void UpdateLightTransforms(const std::vector<LightTransform>& lights, DirectX::XMFLOAT4X4* LightTransforms);

    void BuildLights();
    void BuildRenderAssets();
    void BuildRenderPasses();
    void BuildTextures();
    void BuildMesh();
    void BuildRootSignature();
    void BuildSsaoRootSignature();
    void BuildDescriptorHeaps();
    void BuildShadersAndInputLayout();
    void BuildGenericGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildLevel();
   
    void SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& PointLights, const std::vector<Light>& SpotLights);
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawSceneToShadowMap();
    void DrawNormalsAndDepth();

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index)const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const;

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    DynamicLights dynamicLights;

    POINT mLastMousePos;
    Camera mCamera;
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

    std::unordered_map<std::string, std::shared_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::shared_ptr<Texture>> mTextures;
    std::unordered_map<std::string, std::shared_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::shared_ptr<Skeleton>> mSkeletons;
    std::unordered_map<std::string, std::shared_ptr<Animation>> mAnimations;
    std::unordered_map<std::string, std::shared_ptr<AnimationController>> mAnimationControllers;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mMesh;
    std::unordered_map<std::string, std::shared_ptr<TransformNode>> mTransforms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>> mSubsets;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
    std::vector<std::shared_ptr<ModelMaterial>> mMats;
    std::vector<std::string> mTextureNames;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

    std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    UINT mSkyTexHeapIndex = 0;
    UINT mShadowMapHeapIndex = 0;
    UINT mShadowMapHeapIndex2 = 0;
    UINT mSsaoHeapIndexStart = 0;
    UINT mSsaoAmbientMapIndex = 0;
    UINT mNullCubeSrvIndex = 0;
    UINT mNullTexSrvIndex1 = 0;
    UINT mNullTexSrvIndex2 = 0;
    UINT mSkinnedSrvHeapStart = 0;
    UINT SkinnedCBIndex = 0;
    UINT ObjectCBIndex = 0;

    PassConstants mMainPassCB;
    std::vector<PassConstants> mShadowPassCBs;
    std::vector<std::unique_ptr<ShadowMap>> mShadowMaps;

    std::unique_ptr<Ssao> mSsao;
    DirectX::BoundingSphere mSceneBounds;
};