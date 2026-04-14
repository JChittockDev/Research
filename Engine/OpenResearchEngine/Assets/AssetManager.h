// Assets/AssetManager.h
#pragma once

#include "../D3D12/D3DUtil.h"
#include "../Common/Math.h"
#include "../Common/UploadBuffer.h"
#include "../Render/RenderPassConstantBuffers.h"
#include "../Render/Components/Mesh.h"
#include "../Render/RenderItem.h"
#include "../Render/Components/Skinning.h"
#include "../Models/Internal/GeometryGenerator.h"
#include "../Serialize/LevelReader.h"

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
    void Build(
        const DynamicLights& lights,
        const std::unordered_map<std::string,
              std::unordered_map<std::string, ItemData>>& levelItems,
        const std::unordered_map<std::string,
              std::unordered_map<std::string, PBRMaterialData>>& levelMaterials,
        const std::unordered_map<std::string,
              std::unordered_map<std::string, LightData>>& levelLights
    );

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
    std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>                 mGeometries;
    std::unordered_map<std::string, std::shared_ptr<Skeleton>>                     mSkeletons;
    std::unordered_map<std::string, std::shared_ptr<Animation>>                    mAnimations;
    std::unordered_map<std::string, std::shared_ptr<SkinningController>>           mSkinningControllers;
    std::unordered_map<std::string, std::shared_ptr<BlendshapeController>>         mBlendshapeControllers;
    std::unordered_map<std::string, std::shared_ptr<MeshAnimationResource>>        mMeshAnimationResources;
    std::unordered_map<std::string, std::shared_ptr<Mesh>>                         mMesh;
    std::unordered_map<std::string, std::shared_ptr<TransformNode>>                mTransforms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>>          mSubsets;
    std::map<std::string, std::string>                                             mTextureData;
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

    // Build sub-methods
    void CompileShaders(const DynamicLights& lights);
    void SetRootSignatures();
    void SetPipelineStates();
    void ImportTextures(
        const std::unordered_map<std::string,
              std::unordered_map<std::string, PBRMaterialData>>& levelMaterials
    );
    void PushGenericMesh();
    void PushMesh(
        const std::unordered_map<std::string,
              std::unordered_map<std::string, ItemData>>& levelItems
    );
    void PushMaterials(
        const std::unordered_map<std::string,
              std::unordered_map<std::string, PBRMaterialData>>& levelMaterials
    );

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
