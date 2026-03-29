#include "RenderPipeline.h"

void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
{
    mPasses.push_back(std::move(pass));
}

void RenderPipeline::Execute(const RenderContext& ctx, FrameResource* fr)
{
    for (auto& pass : mPasses)
        pass->Execute(ctx, fr);
}
