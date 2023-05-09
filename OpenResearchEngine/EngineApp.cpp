#include "EngineApp.h"

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
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool EngineApp::Initialize()
{
    if(!D3DApp::Initialize()) 
    { 
        return false; 
    }

    this->mMainWndCaption = L"GRAPE";
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.SetPosition(0.0f, 2.0f, 15.0f);
    mCamera.RotateY(3.25f);

    BuildScene();
    SetRenderPassResources();
    
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    FlushCommandQueue();

    return true;
}
 
void EngineApp::OnResize()
{
    D3DApp::OnResize();
	mCamera.SetLens(0.25f*Math::Pi, AspectRatio(), 1.0f, 1000.0f);

    if (mSsao != nullptr)
    {
        mSsao->OnResize(mClientWidth, mClientHeight);
        mSsao->RebuildDescriptors(mDepthStencilBuffer.Get());
    }
}

void EngineApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);

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
}

void EngineApp::Draw(const GameTimer& gt)
{
    Render(mCurrFrameResource);

    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
    mCurrFrameResource->Fence = ++mCurrentFence;
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}
 
CD3DX12_CPU_DESCRIPTOR_HANDLE EngineApp::GetCpuSrv(int index)const
{
    auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    srv.Offset(index, mCbvSrvUavDescriptorSize);
    return srv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE EngineApp::GetGpuSrv(int index)const
{
    auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    srv.Offset(index, mCbvSrvUavDescriptorSize);
    return srv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE EngineApp::GetDsv(int index)const
{
    auto dsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvHeap->GetCPUDescriptorHandleForHeapStart());
    dsv.Offset(index, mDsvDescriptorSize);
    return dsv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE EngineApp::GetRtv(int index)const
{
    auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtv.Offset(index, mRtvDescriptorSize);
    return rtv;
}