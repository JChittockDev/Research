#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Objects/Object.h"
#include "Models/External/LoadM3d.h"
#include "Models/Internal/GeometryGenerator.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Passes/ShadowMap.h"
#include "Render/Passes/Ssao.h"
#include "Render/Resources/SkinnedData.h"
#include "Render/Resources/RenderItem.h"

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
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateSkinnedCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
    void UpdateSsaoCB(const GameTimer& gt);

    void LoadTextures();
    void BuildRootSignature();
    void BuildSsaoRootSignature();
    void BuildDescriptorHeaps();
    void BuildShadersAndInputLayout();
    
    std::unique_ptr<MeshGeometry> GenericGeometry();
    std::unique_ptr<MeshMaterial> GenericMaterials();
    std::unordered_map<std::string, std::string> GenericTextures();

    void BuildMesh(std::unordered_map<std::string, std::pair<MeshType, std::string>>& inputData,
                    std::unordered_map<std::string, std::shared_ptr<MeshRenderData>>& meshRenderAssets);
    void BuildPSOs();
    void BuildFrameResources();
    void BuildScene();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawSceneToShadowMap();
    void DrawNormalsAndDepth();
    void SerializeAssets();

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index)const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const;

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    UINT objCBIndex = 0;
    UINT matCBIndex = 0;
    UINT srvHeapIndex = 0;
    UINT skinnedCBIndex = 0;

    std::unordered_map<std::string, std::shared_ptr<MeshRenderData>> meshRenderAssets;
    std::unordered_map<std::string, std::pair<MeshType, std::string>> meshDefinitions;

    std::unordered_map<std::string, std::string> textureDefinitions;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textureRenderAssets;

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    // redundant at this stage, will use later;
    //std::unordered_map<std::string, std::shared_ptr<MeshGeometry>> mGeometries;
    //std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

    std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    std::unordered_map<RenderLayer, std::vector<RenderItem*>> mRitemLayer;

    UINT mSkyTexHeapIndex = 0;
    UINT mShadowMapHeapIndex = 0;
    UINT mSsaoHeapIndexStart = 0;
    UINT mSsaoAmbientMapIndex = 0;

    UINT mNullCubeSrvIndex = 0;
    UINT mNullTexSrvIndex1 = 0;
    UINT mNullTexSrvIndex2 = 0;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

    PassConstants mMainPassCB;  // index 0 of pass cbuffer.
    PassConstants mShadowPassCB;// index 1 of pass cbuffer.

    Camera mCamera;

    std::unique_ptr<ShadowMap> mShadowMap;
    std::unique_ptr<Ssao> mSsao;
    DirectX::BoundingSphere mSceneBounds;

    float mLightNearZ = 0.0f;
    float mLightFarZ = 0.0f;
    XMFLOAT3 mLightPosW;
    XMFLOAT4X4 mLightView = Math::Identity4x4();
    XMFLOAT4X4 mLightProj = Math::Identity4x4();
    XMFLOAT4X4 mShadowTransform = Math::Identity4x4();

    float mLightRotationAngle = 0.0f;
    XMFLOAT3 mBaseLightDirections[3] = {
        XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(0.0f, -0.707f, -0.707f)
    };
    XMFLOAT3 mRotatedLightDirections[3];

    POINT mLastMousePos;
};