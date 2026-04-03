#include "../../EngineApp.h"
#include "../RenderContext.h"

void EngineApp::Render(FrameResource* currentFrameResource)
{
    ThrowIfFailed(currentFrameResource->CmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(currentFrameResource->CmdListAlloc.Get(),
        mAssets->mPSOs.at("GBuffer").Get()));

    ID3D12DescriptorHeap* descriptorHeaps[] = { renderPassSrvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    mOnnxModelResource->Evaluate(std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });

    RenderContext ctx;
    ctx.cmdList          = mCommandList.Get();
    ctx.viewport         = mScreenViewport;
    ctx.scissorRect      = mScreenScissorRect;
    ctx.dsv              = DepthStencilView();
    ctx.backBuffer       = CurrentBackBuffer().Get();
    ctx.backBufferRtv    = CurrentBackBufferView();
    ctx.renderItemLayers = &mRenderItemLayers;
    ctx.lights           = &dynamicLights;

    mPipeline.Execute(ctx, currentFrameResource);
}
