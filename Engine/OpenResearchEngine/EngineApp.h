#pragma once

#include "D3D12/D3DApp.h"
#include "Common/Math.h"
#include "Common/UploadBuffer.h"
#include "Objects/Camera.h"
#include "Models/Internal/GeometryGenerator.h"
#include "Render/Resources/FrameResource.h"
#include "Render/Resources/ShadowMap.h"
#include "Render/Resources/Ssao.h"
#include "Render/Resources/SSS.h"
#include "Render/Resources/Composite.h"
#include "Render/Resources/Lighting.h"
#include "Render/Resources/Ssgi.h"
#include "Render/Resources/ShadowResources.h"
#include "Utilities/OnnxModelResource.h"
#include "Render/Resources/RadianceResources.h"
#include "Render/Resources/GBuffer.h"
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/Skinning.h"
#include "Render/Resources/RenderTextures.h"
#include "Serialize/LevelReader.h"
#include "Render/Resources/RenderMeshAsset.h"
#include "Render/Resources/SimMeshAsset.h"
#include "Render/Resources/MeshInstance.h"
#include "Render/Deformers/SkinDeformer.h"
#include "Render/Deformers/BlendshapeDeformer.h"
#include "Render/Deformers/PhysicsDeformer.h"
#include "Render/Deformers/DeformationGraph.h"
#include "Render/Resources/StaticBatch.h"

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
    void UpdateScreenSpaceCB(const GameTimer& gt);
    void UpdateSssCB(const GameTimer& gt);
    void UpdateRadiancePassCB(const GameTimer& gt);

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
    void SerializeLevel();
    void SetRenderPassResources();
    void BuildScene();

    void Render(FrameResource* currentFrameResource);

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

    void SetRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>>& renderItems, FrameResource* currentFrameResource);
    void ShadowPass(const DynamicLights& lights, FrameResource* currentFrameResource);
    void DeformationPass(FrameResource* currentFrameResource);
    void GBufferPass(FrameResource* currentFrameResource);
    void LightingPass(FrameResource* currentFrameResource);
    void CompositePass(FrameResource* currentFrameResource);
    void SsaoPass(FrameResource* currentFrameResource);
    void SsaoBlurPass(FrameResource* currentFrameResource);
    void SsgiPass(FrameResource* currentFrameResource);
    void SsgiBlurPass(FrameResource* currentFrameResource);
    void SssPass(FrameResource* currentFrameResource);
    void SssBlurPass(FrameResource* currentFrameResource);
    void RadiancePass(const DynamicLights& lights, FrameResource* currentFrameResource);

    void SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms);

    std::string extractFileName(const std::string& filePath);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GetStaticSamplers();

    PhysicsDeformerResources MakePhysicsDeformerResources();

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
    ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSsgiRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSssRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mEdgeBlurRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mColorEdgeBlurRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mCompositeRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mRadianceRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
    ComPtr<ID3D12RootSignature> mPoissonBlurRootSignature = nullptr;

    ComPtr<ID3D12RootSignature> mGemmRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mReluRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mLeakyReluRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSigmoidRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mTanhRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSoftmaxRootSignature = nullptr;

    DynamicLights dynamicLights;
    std::unordered_map<std::string, std::shared_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>> mTextures;
    std::unordered_map<std::string, std::shared_ptr<Skeleton>> mSkeletons;
    std::unordered_map<std::string, std::shared_ptr<Animation>> mAnimations;
    std::unordered_map<std::string, std::shared_ptr<SkinningController>> mSkinningControllers;
    std::unordered_map<std::string, std::shared_ptr<BlendshapeController>> mBlendshapeControllers;
    std::unordered_map<std::string, std::shared_ptr<TransformNode>> mTransforms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subset>>> mSubsets;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
    std::map<std::string, std::string> mTextureData;

    // New mesh system (replaces mGeometries, mMesh, mMeshAnimationResources etc.)
    std::unordered_map<std::string, std::shared_ptr<RenderMeshAsset>> mRenderMeshAssets;
    std::unordered_map<std::string, std::shared_ptr<SimMeshAsset>>    mSimMeshAssets;
    std::unordered_map<std::string, std::shared_ptr<MeshInstance>>    mMeshInstances;
    std::vector<SkinDeformer*>        mSkinDeformers;       // flat list, index = SkinnedCB slot
    std::vector<BlendshapeDeformer*>  mBlendshapeDeformers; // flat list, index = BlendCB slot
    std::vector<std::unique_ptr<StaticBatch>> mStaticBatches;

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

    std::vector<RadianceConstants> mRadianceCBs;

	std::unique_ptr<OnnxModelResource> mOnnxModelResource;

    std::unique_ptr<GBuffer> mGBuffer;
    std::unique_ptr<Lighting> mLighting;
    std::unique_ptr<Ssao> mSsao;
    std::unique_ptr<Ssgi> mSsgi;
    std::unique_ptr<SSS> mSss;
    std::unique_ptr<Composite> mComposite;
    std::unique_ptr<RenderTextures> mRenderTextures;
    std::unique_ptr<ShadowResources> mShadowResources;
    std::unique_ptr<RadianceResources> mRadianceResources;

    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap = nullptr;
};