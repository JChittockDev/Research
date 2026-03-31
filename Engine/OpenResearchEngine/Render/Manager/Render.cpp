#include "../../EngineApp.h"
#include "../../Render/RenderContext.h"
#include <unordered_set>

void EngineApp::Render(FrameResource* fr)
{
    ThrowIfFailed(fr->CmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(fr->CmdListAlloc.Get(), nullptr));

    ID3D12DescriptorHeap* descriptorHeaps[] = { renderPassSrvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    mAssets->mOnnxModelResource->Evaluate(std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });

    mSceneState.clientWidth  = mClientWidth;
    mSceneState.clientHeight = mClientHeight;

    RenderContext ctx;
    ctx.cmdList          = mCommandList.Get();
    ctx.viewport         = mScreenViewport;
    ctx.scissorRect      = mScreenScissorRect;
    ctx.dsv              = DepthStencilView();
    ctx.backBuffer       = CurrentBackBuffer().Get();
    ctx.backBufferRtv    = CurrentBackBufferView();
    ctx.renderItemLayers = &mAssets->mRenderItemLayers;
    ctx.lights           = &mSceneState.lights;
    ctx.assets           = mAssets.get();
    ctx.sceneState       = &mSceneState;

    mRenderPipeline->Execute(ctx, fr);
}
