#include "EngineApp.h"
#include "Render/Passes/AnimationPass.h"
#include "Render/Passes/PhysicsPass.h"
#include "Render/Passes/ShadowPass.h"
#include "Render/Passes/GBufferPass.h"
#include "Render/Passes/SsaoPass.h"
#include "Render/Passes/SsaoBlurPass.h"
#include "Render/Passes/RadiancePass.h"
#include "Render/Passes/SssPass.h"
#include "Render/Passes/SssBlurPass.h"
#include "Render/Passes/LightingPass.h"
#include "Render/Passes/SsgiPass.h"
#include "Render/Passes/SsgiBlurPass.h"
#include "Render/Passes/CompositePass.h"
#include "Update/UpdateFunctions.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
const int gNumFrameResources = 3;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        EngineApp theApp(hInstance);
        
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

EngineApp::EngineApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
}

EngineApp::~EngineApp()
{
    if (md3dDevice != nullptr)
    {
        FlushCommandQueue();
    }

    ImGui_ImplDX12_Shutdown();
}

bool EngineApp::Initialize()
{
    if(!D3DApp::Initialize()) 
    { 
        return false; 
    }

    this->mMainWndCaption = L"OpenResearchEngine";
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.SetPosition(0.0f, 2.0f, 15.0f);
    mCamera.RotateY(3.25f);

    BuildScene();
    SetRenderPassResources();

    // AnimationSystem
    mAnimationSystem = std::make_unique<AnimationSystem>();

    // RenderPipeline — one pass per stage in submission order
    mRenderPipeline = std::make_unique<RenderPipeline>();

    AnimationPassResources animRes;
    animRes.blend      = mAssets->mBlendRootSignature.Get();
    animRes.skinned    = mAssets->mSkinnedRootSignature.Get();
    animRes.psoBlend   = mAssets->mPSOs.at("blend").Get();
    animRes.psoSkinned = mAssets->mPSOs.at("skinned").Get();
    mRenderPipeline->AddPass(std::make_unique<AnimationPass>(animRes));

    PhysicsPassResources physRes;
    physRes.meshTransfer    = mAssets->mMeshTransferRootSignature.Get();
    physRes.simMeshTransfer = mAssets->mSimMeshTransferRootSignature.Get();
    physRes.triangleNormal  = mAssets->mTriangleNormalRootSignature.Get();
    physRes.vertexNormal    = mAssets->mVertexNormalRootSignature.Get();
    physRes.force           = mAssets->mForceRootSignature.Get();
    physRes.preSolve        = mAssets->mPreSolveRootSignature.Get();
    physRes.postSolve       = mAssets->mPostSolveRootSignature.Get();
    physRes.constraintSolve = mAssets->mConstraintSolveRootSignature.Get();
    physRes.tension         = mAssets->mTensionRootSignature.Get();
    physRes.psoMeshTransfer    = mAssets->mPSOs.at("meshTransfer").Get();
    physRes.psoSimMeshTransfer = mAssets->mPSOs.at("simMeshTransfer").Get();
    physRes.psoTriangleNormal  = mAssets->mPSOs.at("triangleNormal").Get();
    physRes.psoVertexNormal    = mAssets->mPSOs.at("vertexNormal").Get();
    physRes.psoForce           = mAssets->mPSOs.at("force").Get();
    physRes.psoPreSolve        = mAssets->mPSOs.at("preSolve").Get();
    physRes.psoPostSolve       = mAssets->mPSOs.at("postSolve").Get();
    physRes.psoConstraintSolve = mAssets->mPSOs.at("constraintSolve").Get();
    physRes.psoTension         = mAssets->mPSOs.at("tension").Get();
    mRenderPipeline->AddPass(std::make_unique<PhysicsPass>(physRes));

    mRenderPipeline->AddPass(std::make_unique<ShadowPass>(
        mAssets->mShadowsRootSignature.Get(),
        mAssets->mPSOs.at("shadow_opaque").Get(),
        mShadowResources.get(), mRenderTextures.get()));

    mRenderPipeline->AddPass(std::make_unique<GBufferPass>(
        mAssets->mGBufferRootSignature.Get(),
        mAssets->mPSOs.at("GBuffer").Get(),
        mGBuffer.get(), mRenderTextures.get()));

    mRenderPipeline->AddPass(std::make_unique<SsaoPass>(
        mAssets->mSsaoRootSignature.Get(),
        mAssets->mPSOs.at("Ssao").Get(),
        mGBuffer.get(), mSsao.get()));

    mRenderPipeline->AddPass(std::make_unique<SsaoBlurPass>(
        mAssets->mEdgeBlurRootSignature.Get(),
        mAssets->mPSOs.at("EdgeBlur").Get(),
        mGBuffer.get(), mSsao.get()));

    mRenderPipeline->AddPass(std::make_unique<RadiancePass>(
        mAssets->mRadianceRootSignature.Get(),
        mAssets->mPSOs.at("Radiance").Get(),
        mGBuffer.get(), mShadowResources.get(), mRadianceResources.get()));

    mRenderPipeline->AddPass(std::make_unique<SssPass>(
        mAssets->mSssRootSignature.Get(),
        mAssets->mPSOs.at("Sss").Get(),
        mGBuffer.get(), mSss.get(), mRadianceResources.get()));

    mRenderPipeline->AddPass(std::make_unique<SssBlurPass>(
        mAssets->mPoissonBlurRootSignature.Get(),
        mAssets->mPSOs.at("PoissonBlur").Get(),
        mSss.get()));

    mRenderPipeline->AddPass(std::make_unique<LightingPass>(
        mAssets->mLightingRootSignature.Get(),
        mAssets->mPSOs.at("Lighting").Get(),
        mGBuffer.get(), mSss.get(), mRadianceResources.get(), mLighting.get()));

    mRenderPipeline->AddPass(std::make_unique<SsgiPass>(
        mAssets->mSsgiRootSignature.Get(),
        mAssets->mPSOs.at("Ssgi").Get(),
        mLighting.get(), mGBuffer.get(), mSsgi.get()));

    mRenderPipeline->AddPass(std::make_unique<SsgiBlurPass>(
        mAssets->mColorEdgeBlurRootSignature.Get(),
        mAssets->mPSOs.at("ColorEdgeBlur").Get(),
        mGBuffer.get(), mSsgi.get()));

    mRenderPipeline->AddPass(std::make_unique<CompositePass>(
        mAssets->mCompositeRootSignature.Get(),
        mAssets->mPSOs.at("Composite").Get(),
        mLighting.get(), mSsgi.get()));

    ImGui_ImplDX12_Init(md3dDevice.Get(), 2, DXGI_FORMAT_R8G8B8A8_UNORM, imGuiSrvDescriptorHeap.Get(), imGuiSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), imGuiSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    FlushCommandQueue();

    return true;
}
 
void EngineApp::OnResize()
{
    D3DApp::OnResize();
	mCamera.SetLens(0.25f*Math::Pi, AspectRatio(), 0.001f, 1000.0f);

    if (mGBuffer != nullptr)
    {
        mGBuffer->OnResize(mClientWidth, mClientHeight);
        mGBuffer->RebuildDescriptors();
    }

    if (mSsao != nullptr)
    {
        mSsao->OnResize(mClientWidth, mClientHeight);
        mSsao->RebuildDescriptors(GetDepthBuffer());
    }

    if (mLighting != nullptr)
    {
        mLighting->OnResize(mClientWidth, mClientHeight);
        mLighting->RebuildDescriptors();
    }

    if (mRadianceResources != nullptr)
    {
        for (int i = 0; i < mRadianceResources->radianceMaps.size(); i++)
        {
            if (mRadianceResources->radianceMaps[i] != nullptr)
            {
                mRadianceResources->radianceMaps[i]->OnResize(mClientWidth, mClientHeight);
            }
        }

        for (int i = 0; i < mRadianceResources->radianceMaps.size(); i++)
        {
            if (mRadianceResources->radianceMaps[i] != nullptr)
            {
                mRadianceResources->radianceMaps[i]->RebuildDiffuseDescriptors();
            }
        }

        for (int i = 0; i < mRadianceResources->radianceMaps.size(); i++)
        {
            if (mRadianceResources->radianceMaps[i] != nullptr)
            {
                mRadianceResources->radianceMaps[i]->RebuildSpecularDescriptors();
            }
        }
    }

    if (mSsgi != nullptr)
    {
        mSsgi->OnResize(mClientWidth, mClientHeight);
        mSsgi->RebuildDescriptors(GetDepthBuffer());
    }

    if (mSss != nullptr)
    {
        mSss->OnResize(mClientWidth, mClientHeight);
        mSss->RebuildDescriptors(GetDepthBuffer());
    }

    if (mComposite != nullptr)
    {
        mComposite->OnResize(mClientWidth, mClientHeight);
        mComposite->RebuildDescriptors();
    }
}

void EngineApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
    if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    mSceneState.clientWidth  = mClientWidth;
    mSceneState.clientHeight = mClientHeight;

    ImGui::Begin("Scene");
    UpdateLights         (gt, mSceneState, *mAssets, mCurrFrameResource);
    UpdateObjectCBs      (gt, *mAssets, mCurrFrameResource);
    mAnimationSystem->Update(gt, *mAssets, *mCurrFrameResource);
    UpdateMaterialBuffer (gt, *mAssets, mCurrFrameResource);
    UpdateShadowTransform(gt, mSceneState);
    UpdateShadowPassCB   (gt, mSceneState, mShadowResources.get(), mCurrFrameResource);
    UpdateMainPassCB     (gt, mCamera, mSceneState, mCurrFrameResource);
    UpdateSssCB          (gt, mSceneState, mCurrFrameResource);
    UpdateScreenSpaceCB  (gt, mSceneState, mSsao.get(), mSsgi.get(), mSss.get(), mCurrFrameResource);
    UpdateRadiancePassCB (gt, mSceneState, *mAssets, mCurrFrameResource);
    ImGui::End();

    static bool show = false;
    if (show) ImGui::ShowDemoWindow(&show);
}

void EngineApp::Draw(const GameTimer& gt)
{
    Render(mCurrFrameResource);

    ImGui::Render();
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Render Dear ImGui graphics
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    //mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clear_color_with_alpha, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &(CurrentBackBufferView()), FALSE, nullptr);

    ID3D12DescriptorHeap* descriptorHeaps[] = { imGuiSrvDescriptorHeap.Get() };

    mCommandList->SetDescriptorHeaps(1, descriptorHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    mAssets->mOnnxModelResource->ReadBackOutput();

    mCommandList->Close();

    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
    mCurrFrameResource->Fence = ++mCurrentFence;
   
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);

	// Test model output retrieval
    std::vector<float> output = mAssets->mOnnxModelResource->GetOutputData();
}

void EngineApp::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1 + MaxLights;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC renderPassRtvHeapDesc = {};
	renderPassRtvHeapDesc.NumDescriptors = 64;
	renderPassRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderPassRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	renderPassRtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&renderPassRtvHeapDesc, IID_PPV_ARGS(renderPassRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC renderPassSrvHeapDesc = {};
	renderPassSrvHeapDesc.NumDescriptors = 64;
	renderPassSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	renderPassSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	renderPassSrvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&renderPassSrvHeapDesc, IID_PPV_ARGS(renderPassSrvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC imguiDesc = {};
	imguiDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	imguiDesc.NumDescriptors = 1;
	imguiDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&imguiDesc, IID_PPV_ARGS(&imGuiSrvDescriptorHeap)));
}

void EngineApp::BuildScene()
{
    mAssets = std::make_unique<AssetManager>(
        md3dDevice, mCommandList, GetDirectoryPath(),
        m4xMsaaState, m4xMsaaQuality, mBackBufferFormat, mDepthStencilFormat);
    mAssets->Build();

    mFrameResources.clear();
    for (int i = 0; i < gNumFrameResources; ++i)
        mFrameResources.push_back(std::make_shared<FrameResource>(
            md3dDevice.Get(),
            mAssets->dynamicLights.GetNumLights(),
            (UINT)mAssets->mRenderItems.size(),
            1, 1,
            (UINT)mAssets->mMaterials.size()));

    mSceneState.lights       = mAssets->dynamicLights;
    mSceneState.sceneBounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    mSceneState.sceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);
    mSceneState.shadowPassCBs.resize(mAssets->dynamicLights.GetNumLights());
    mSceneState.radianceCBs.resize(mAssets->dynamicLights.GetNumLights());
}

void EngineApp::SetRenderPassResources()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle(renderPassRtvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle(renderPassSrvHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle(renderPassSrvHeap.Get()->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle(mDsvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    mGBuffer = std::make_unique<GBuffer>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mGBuffer->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSsao = std::make_unique<Ssao>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsao->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mLighting = std::make_unique<Lighting>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mLighting->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSsgi = std::make_unique<Ssgi>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsgi->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mSss = std::make_unique<SSS>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSss->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mComposite = std::make_unique<Composite>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mComposite->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);

    mRadianceResources = std::make_unique<RadianceResources>(md3dDevice.Get());
    mRadianceResources->BuildDescriptors(mAssets->dynamicLights.GetNumLights(), mClientWidth, mClientHeight, cpuSrvHandle, gpuSrvHandle, cpuRtvHandle, mCbvSrvUavDescriptorSize, mRtvDescriptorSize);

    mRenderTextures = std::make_unique<RenderTextures>(md3dDevice.Get());
    mRenderTextures->BuildDescriptors(cpuSrvHandle, gpuSrvHandle, mCbvSrvUavDescriptorSize, mAssets->mTextureData, mAssets->mTextures);

    cpuDsvHandle = cpuDsvHandle.Offset(1, mDsvDescriptorSize);
    mShadowResources = std::make_unique<ShadowResources>(md3dDevice.Get());
    mShadowResources->BuildDescriptors(mAssets->dynamicLights.GetNumLights(), cpuSrvHandle, gpuSrvHandle, cpuDsvHandle, mCbvSrvUavDescriptorSize, mDsvDescriptorSize);
}

