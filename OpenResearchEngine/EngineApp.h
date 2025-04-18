#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Models/Internal/GeometryGenerator.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/Mesh.h"
#include "Render/Resources/ShadowMap.h"
#include "Render/Resources/ShadowResources.h"
#include "Render/Resources/GBuffer.h"
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/Skinning.h"
#include "Render/Resources/RenderTextures.h"
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
    void UpdateAnimCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
    void UpdateLights(const GameTimer& gt);
    void UpdateLightTransforms(const std::vector<LightTransform>& lights, DirectX::XMFLOAT4X4* LightTransforms);

    void PushLights();
    void PushMesh();
    void PushGenericMesh();
    void PushMaterials();
    void PushRenderItems();
    void ImportTextures();
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
    void ComputeSkinning(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeBlendshapes(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeTriangleNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeVertexNormals(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeSimMeshTransfer(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputePreSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputePostSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeConstraintSolve(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputePBD(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeForce(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);
    void ComputeTension(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<RenderItem>& ri, FrameResource* currentFrameResource);

    void SetBlendRootSignature();
    void SetSkinnedRootSignature();
    void SetTriangleNormalRootSignature();
    void SetVertexNormalRootSignature();
    void SetSimMeshTransferRootSignature();
    void SetPreSolveRootSignature();
    void SetPostSolveRootSignature();
    void SetConstraintSolveRootSignature();
    void SetMeshTransferRootSignature();
    void SetForceRootSignature();
    void SetTensionRootSignature();
    void SetGBufferRootSignature();
    void SetLightingRootSignature();
    void SetShadowsRootSignature();

    void SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource);
    void ShadowPass(const DynamicLights& lights, FrameResource* currentFrameResource);
    void DeformationPass(FrameResource* currentFrameResource);
    void GBufferPass(FrameResource* currentFrameResource);
    void LightingPass(FrameResource* currentFrameResource);

    void SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms);

    std::string extractFileName(const std::string& filePath);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
    POINT mLastMousePos;
    Camera mCamera;

    DirectX::BoundingSphere mSceneBounds;
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mBlendRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSkinnedRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mTensionRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mForceRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mPreSolveRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mPostSolveRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mConstraintSolveRootSignature = nullptr;

    ComPtr<ID3D12RootSignature> mTriangleNormalRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mVertexNormalRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mMeshTransferRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSimMeshTransferRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mGBufferRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mShadowsRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mLightingRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    DynamicLights dynamicLights;
    std::unordered_map<std::string, std::shared_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>> mTextures;
    std::unordered_map<std::string, std::shared_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::shared_ptr<Skeleton>> mSkeletons;
    std::unordered_map<std::string, std::shared_ptr<Animation>> mAnimations;
    std::unordered_map<std::string, std::shared_ptr<SkinningController>> mSkinningControllers;
    std::unordered_map<std::string, std::shared_ptr<BlendshapeController>> mBlendshapeControllers;
    std::unordered_map<std::string, std::shared_ptr<MeshAnimationResource>> mMeshAnimationResources;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mMesh;
    std::unordered_map<std::string, std::shared_ptr<TransformNode>> mTransforms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>> mSubsets;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
    std::map<std::string, std::string> mTextureData;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

    int mCurrFrameResourceIndex = 0;
    FrameResource* mCurrFrameResource = nullptr;
    std::vector<std::shared_ptr<RenderItem>> mRenderItems;
    std::vector<std::shared_ptr<FrameResource>> mFrameResources;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mMeshRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mDeformedRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mDirectionalLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mSpotLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>> mRenderItemLayers;
    std::unordered_map<std::string, std::unordered_map<std::string, ItemData>> mLevelRenderItems;
    std::unordered_map<std::string, std::unordered_map<std::string, PBRMaterialData>> mLevelMaterials;
    std::unordered_map<std::string, std::unordered_map<std::string, LightData>> mLevelLights;

    UINT BlendCBIndex = 0;
    UINT SkinnedCBIndex = 0;
    UINT ObjectCBIndex = 0;

    PassConstants mMainPassCB;
    std::vector<PassConstants> mShadowPassCBs;
    std::vector<std::unique_ptr<ShadowMap>> mShadowMaps;

    std::unique_ptr<GBuffer> mGBuffer;
    std::unique_ptr<RenderTextures> mRenderTextures;
    std::unique_ptr<ShadowResources> mShadowResources;

    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap = nullptr;
};