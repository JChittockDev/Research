#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Models/Internal/GeometryGenerator.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/Mesh.h"
#include "Render/Resources/ShadowMap.h"
#include "Render/Resources/Ssao.h"
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

    void PushLights();
    void PushMesh();
    void PushGenericMesh();
    void PushMaterials();
    void PushRenderItems();
    void ImportTextures();
    void PopulateDescriptorHeaps();
    void CompileShaders();
    void SetPipelineStates();
    void SetFenceResources();
    void SetRootSignatures();
    void SetGenericRootSignature();
    void SetSsaoRootSignature();
    void SerializeLevel();
    void SetRenderPassResources();
    void BuildScene();

    void Render(FrameResource* currentFrameResource);
    void SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource);
    void ShadowPass(const DynamicLights& lights, FrameResource* currentFrameResource);
    void DepthPass(const D3D12_CPU_DESCRIPTOR_HANDLE& dsv, FrameResource* currentFrameResource);
    void SsaoPass(int blurCount, FrameResource* currentFrameResource);
    void DiffusePass(const std::unordered_map<std::string, std::pair<INT, UINT>>& layoutIndexMap, FrameResource* currentFrameResource);
    void SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& PointLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms);
    
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& ritems);
    void DrawSceneToShadowMap();
    void DrawNormalsAndDepth();

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index)const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const;

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    POINT mLastMousePos;
    Camera mCamera;

    DirectX::BoundingSphere mSceneBounds;
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

    DynamicLights dynamicLights;
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

    int mCurrFrameResourceIndex = 0;
    FrameResource* mCurrFrameResource = nullptr;
    std::vector<std::shared_ptr<RenderItem>> mRenderItems;
    std::vector<std::shared_ptr<FrameResource>> mFrameResources;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mMeshRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mRenderItemLayers;
    std::unordered_map<std::string, std::unordered_map<std::string, ItemData>> mLevelRenderItems;
    std::unordered_map<std::string, std::pair<INT, UINT>> mLayoutIndicies;

    UINT SkinnedCBIndex = 0;
    UINT ObjectCBIndex = 0;

    PassConstants mMainPassCB;
    std::unique_ptr<Ssao> mSsao;
    std::vector<PassConstants> mShadowPassCBs;
    std::vector<std::unique_ptr<ShadowMap>> mShadowMaps;
};