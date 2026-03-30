// Assets/AssetManager.h
#pragma once

#include "../D3D12/D3DUtil.h"
#include "../Common/Math.h"
#include "../Common/UploadBuffer.h"
#include "../Render/Resources/FrameResource.h"
#include "../Render/Resources/RenderItem.h"
#include "../Render/Resources/Skinning.h"
#include "../Models/Internal/GeometryGenerator.h"
#include "../Serialize/LevelReader.h"
#include "../Render/Resources/RenderMeshAsset.h"
#include "../Render/Resources/SimMeshAsset.h"
#include "../Render/Resources/MeshInstance.h"
#include "../Render/Deformers/SkinDeformer.h"
#include "../Render/Deformers/BlendshapeDeformer.h"
#include "../Render/Resources/StaticBatch.h"
#include "../Utilities/OnnxModelResource.h"

using Microsoft::WRL::ComPtr;

class AssetManager {
public:
    AssetManager(
        Microsoft::WRL::ComPtr<ID3D12Device> device,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
        std::filesystem::path basePath,
        bool msaaState,
        UINT msaaQuality,
        DXGI_FORMAT backBufferFormat,
        DXGI_FORMAT depthStencilFormat
    );

    // Top-level init — called once from EngineApp::BuildScene()
    void Build();

    // Path helper (mirrors D3DApp::GetFullPath)
    std::string GetFullPath(const char* path) const {
        return (mBasePath / std::filesystem::path(path)).string();
    }
    bool PathExists(const std::string& path) const {
        return std::filesystem::exists(GetFullPath(path.c_str()));
    }
    std::string ChangeExtension(std::string path, std::string ext) const {
        return std::filesystem::path(path).replace_extension(ext).string();
    }

    // --- Resource Maps (public — accessed by EngineApp Build/ and Update/ methods) ---
    std::unordered_map<std::string, std::shared_ptr<Material>>                     mMaterials;
    std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>>     mTextures;
    std::unordered_map<std::string, std::shared_ptr<Skeleton>>                     mSkeletons;
    std::unordered_map<std::string, std::shared_ptr<Animation>>                    mAnimations;
    std::unordered_map<std::string, std::shared_ptr<SkinningController>>           mSkinningControllers;
    std::unordered_map<std::string, std::shared_ptr<BlendshapeController>>         mBlendshapeControllers;
    std::unordered_map<std::string, std::shared_ptr<TransformNode>>                mTransforms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>          mSubsets;
    std::map<std::string, std::string>                                             mTextureData;

    // Render items (populated by Build, consumed by render passes and Update* functions)
    std::vector<std::shared_ptr<RenderItem>>                                    mRenderItems;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mRenderItemLayers;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mMeshRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mDeformedRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mDirectionalLightRenderItemMap;
    std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mSpotLightRenderItemMap;
    UINT ObjectCBIndex  = 0;
    UINT SkinnedCBIndex = 0;
    UINT BlendCBIndex   = 0;

    // Serialized level data (loaded by SerializeLevel, consumed by Push* methods)
    std::unordered_map<std::string, std::unordered_map<std::string, ItemData>>        mLevelRenderItems;
    std::unordered_map<std::string, std::unordered_map<std::string, PBRMaterialData>> mLevelMaterials;
    std::unordered_map<std::string, std::unordered_map<std::string, LightData>>       mLevelLights;

    // Light state (populated by PushLights, read by SceneState init and UpdateLights)
    DynamicLights dynamicLights;

    // ONNX inference resource
    std::unique_ptr<OnnxModelResource> mOnnxModelResource;

    // New mesh system (replaces mGeometries, mMesh, mMeshAnimationResources)
    std::unordered_map<std::string, std::shared_ptr<RenderMeshAsset>> mRenderMeshAssets;
    std::unordered_map<std::string, std::shared_ptr<SimMeshAsset>>    mSimMeshAssets;
    std::unordered_map<std::string, std::shared_ptr<MeshInstance>>    mMeshInstances;
    std::vector<SkinDeformer*>        mSkinDeformers;
    std::vector<BlendshapeDeformer*>  mBlendshapeDeformers;
    std::vector<std::unique_ptr<StaticBatch>> mStaticBatches;
    std::vector<D3D12_INPUT_ELEMENT_DESC>                                          mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC>                                          mSkinnedInputLayout;

    // --- GPU assets (shaders, PSOs) ---
    std::unordered_map<std::string, ComPtr<ID3DBlob>>             mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>  mPSOs;

    // --- Root Signatures (29 total) ---
    ComPtr<ID3D12RootSignature> mRootSignature;
    ComPtr<ID3D12RootSignature> mBlendRootSignature;
    ComPtr<ID3D12RootSignature> mSkinnedRootSignature;
    ComPtr<ID3D12RootSignature> mTensionRootSignature;
    ComPtr<ID3D12RootSignature> mForceRootSignature;
    ComPtr<ID3D12RootSignature> mPreSolveRootSignature;
    ComPtr<ID3D12RootSignature> mPostSolveRootSignature;
    ComPtr<ID3D12RootSignature> mConstraintSolveRootSignature;
    ComPtr<ID3D12RootSignature> mTriangleNormalRootSignature;
    ComPtr<ID3D12RootSignature> mVertexNormalRootSignature;
    ComPtr<ID3D12RootSignature> mMeshTransferRootSignature;
    ComPtr<ID3D12RootSignature> mSimMeshTransferRootSignature;
    ComPtr<ID3D12RootSignature> mGBufferRootSignature;
    ComPtr<ID3D12RootSignature> mShadowsRootSignature;
    ComPtr<ID3D12RootSignature> mLightingRootSignature;
    ComPtr<ID3D12RootSignature> mSsaoRootSignature;
    ComPtr<ID3D12RootSignature> mSsgiRootSignature;
    ComPtr<ID3D12RootSignature> mSssRootSignature;
    ComPtr<ID3D12RootSignature> mEdgeBlurRootSignature;
    ComPtr<ID3D12RootSignature> mColorEdgeBlurRootSignature;
    ComPtr<ID3D12RootSignature> mCompositeRootSignature;
    ComPtr<ID3D12RootSignature> mRadianceRootSignature;
    ComPtr<ID3D12RootSignature> mPoissonBlurRootSignature;
    ComPtr<ID3D12RootSignature> mGemmRootSignature;
    ComPtr<ID3D12RootSignature> mReluRootSignature;
    ComPtr<ID3D12RootSignature> mLeakyReluRootSignature;
    ComPtr<ID3D12RootSignature> mSigmoidRootSignature;
    ComPtr<ID3D12RootSignature> mTanhRootSignature;
    ComPtr<ID3D12RootSignature> mSoftmaxRootSignature;

private:
    // D3DApp context (non-owning, but stored as ComPtr for API compatibility with Mesh etc.)
    Microsoft::WRL::ComPtr<ID3D12Device>                  mDevice;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>     mCmdList;
    std::filesystem::path          mBasePath;
    bool                           mMsaaState;
    UINT                           mMsaaQuality;
    DXGI_FORMAT                    mBackBufferFormat;
    DXGI_FORMAT                    mDepthStencilFormat;

    // Build sub-methods (all read from members — no parameters needed)
    void SerializeLevel();
    void PushLights();
    void CompileShaders();
    void SetRootSignatures();
    void SetPipelineStates();
    void ImportTextures();
    void PushGenericMesh();
    void PushMesh();
    void PushMaterials();
    void PushRenderItems();
    void MakePhysicsDeformerResources(PhysicsDeformerResources& res);

    // Helpers (moved from EngineApp)
    std::string ExtractFileName(const std::string& filePath);
    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GetStaticSamplers();

    // Individual root signature builders (all called from SetRootSignatures)
    void SetGenericRootSignature();
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
    void SetSssRootSignature();
    void SetTensionRootSignature();
    void SetGBufferRootSignature();
    void SetLightingRootSignature();
    void SetShadowsRootSignature();
    void SetSsaoRootSignature();
    void SetSsgiRootSignature();
    void SetEdgeBlurRootSignature();
    void SetPoissonBlurRootSignature();
    void SetColorEdgeBlurRootSignature();
    void SetCompositeRootSignature();
    void SetRadianceRootSignature();
    void SetGemmRootSignature();
    void SetReluRootSignature();
    void SetLeakyReluRootSignature();
    void SetSigmoidRootSignature();
    void SetTanhRootSignature();
    void SetSoftmaxRootSignature();
};
