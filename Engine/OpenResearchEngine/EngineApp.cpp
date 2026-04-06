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
    mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);
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
    BuildPipeline();

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
        mSsao->SetDepthStencilBuffer(GetDepthBuffer());
        mSsao->RebuildDescriptors();
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

    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
 
    UpdateRenderAssets(gt);

    static bool show = false;
    if (show)
    {
        ImGui::ShowDemoWindow(&show);
    }
}

void EngineApp::Draw(const GameTimer& gt)
{
    Render(mCurrFrameResource);

    ImGui::Render();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Render Dear ImGui graphics
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    //mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clear_color_with_alpha, 0, nullptr);
    mCommandList->OMSetRenderTargets(1, &(CurrentBackBufferView()), FALSE, nullptr);

    ID3D12DescriptorHeap* descriptorHeaps[] = { imGuiSrvDescriptorHeap.Get() };

    mCommandList->SetDescriptorHeaps(1, descriptorHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    mOnnxModelResource->ReadBackOutput();

    mCommandList->Close();

    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
    mCurrFrameResource->Fence = ++mCurrentFence;
   
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);

	// Test model output retrieval
    std::vector<float> output = mOnnxModelResource->GetOutputData();
}

std::string EngineApp::extractFileName(const std::string& filePath) {
    size_t found = filePath.find_last_of("/\\");
    if (found != std::string::npos) {
        return filePath.substr(found + 1);
    }
    return filePath; // If no path separator is found, return the whole path as filename
}

void EngineApp::BuildPipeline()
{
    mPipeline.AddPass(std::make_unique<AnimationPass>(AnimationPassResources{
        mAssets->mBlendRootSignature.Get(),
        mAssets->mSkinnedRootSignature.Get(),
        mAssets->mPSOs.at("blend").Get(),
        mAssets->mPSOs.at("skinned").Get()
    }));

    mPipeline.AddPass(std::make_unique<PhysicsPass>(PhysicsPassResources{
        mAssets->mMeshTransferRootSignature.Get(),
        mAssets->mSimMeshTransferRootSignature.Get(),
        mAssets->mTriangleNormalRootSignature.Get(),
        mAssets->mVertexNormalRootSignature.Get(),
        mAssets->mForceRootSignature.Get(),
        mAssets->mPreSolveRootSignature.Get(),
        mAssets->mPostSolveRootSignature.Get(),
        mAssets->mConstraintSolveRootSignature.Get(),
        mAssets->mTensionRootSignature.Get(),
        mAssets->mPSOs.at("meshTransfer").Get(),
        mAssets->mPSOs.at("simMeshTransfer").Get(),
        mAssets->mPSOs.at("triangleNormal").Get(),
        mAssets->mPSOs.at("vertexNormal").Get(),
        mAssets->mPSOs.at("force").Get(),
        mAssets->mPSOs.at("preSolve").Get(),
        mAssets->mPSOs.at("postSolve").Get(),
        mAssets->mPSOs.at("constraintSolve").Get(),
        mAssets->mPSOs.at("tension").Get()
    }));

    mPipeline.AddPass(std::make_unique<ShadowPass>(
        mAssets->mShadowsRootSignature.Get(),
        mAssets->mPSOs.at("shadow_opaque").Get(),
        mShadowResources.get(),
        mRenderTextures.get()));

    mPipeline.AddPass(std::make_unique<GBufferPass>(
        mAssets->mGBufferRootSignature.Get(),
        mAssets->mPSOs.at("GBuffer").Get(),
        mGBuffer.get(),
        mRenderTextures.get()));

    mPipeline.AddPass(std::make_unique<SsaoPass>(
        mAssets->mSsaoRootSignature.Get(),
        mAssets->mPSOs.at("Ssao").Get(),
        mGBuffer.get(),
        mSsao.get()));

    mPipeline.AddPass(std::make_unique<SsaoBlurPass>(
        mAssets->mEdgeBlurRootSignature.Get(),
        mAssets->mPSOs.at("EdgeBlur").Get(),
        mGBuffer.get(),
        mSsao.get()));

    mPipeline.AddPass(std::make_unique<RadiancePass>(
        mAssets->mRadianceRootSignature.Get(),
        mAssets->mPSOs.at("Radiance").Get(),
        mGBuffer.get(),
        mShadowResources.get(),
        mRadianceResources.get()));

    mPipeline.AddPass(std::make_unique<SssPass>(
        mAssets->mSssRootSignature.Get(),
        mAssets->mPSOs.at("Sss").Get(),
        mGBuffer.get(),
        mSss.get(),
        mRadianceResources.get()));

    mPipeline.AddPass(std::make_unique<SssBlurPass>(
        mAssets->mPoissonBlurRootSignature.Get(),
        mAssets->mPSOs.at("PoissonBlur").Get(),
        mSss.get()));

    mPipeline.AddPass(std::make_unique<LightingPass>(
        mAssets->mLightingRootSignature.Get(),
        mAssets->mPSOs.at("Lighting").Get(),
        mGBuffer.get(),
        mSss.get(),
        mRadianceResources.get(),
        mLighting.get()));

    mPipeline.AddPass(std::make_unique<SsgiPass>(
        mAssets->mSsgiRootSignature.Get(),
        mAssets->mPSOs.at("Ssgi").Get(),
        mLighting.get(),
        mGBuffer.get(),
        mSsgi.get()));

    mPipeline.AddPass(std::make_unique<SsgiBlurPass>(
        mAssets->mColorEdgeBlurRootSignature.Get(),
        mAssets->mPSOs.at("ColorEdgeBlur").Get(),
        mGBuffer.get(),
        mSsgi.get()));

    mPipeline.AddPass(std::make_unique<CompositePass>(
        mAssets->mCompositeRootSignature.Get(),
        mAssets->mPSOs.at("Composite").Get(),
        mLighting.get(),
        mSsgi.get()));
}