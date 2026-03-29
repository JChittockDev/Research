#pragma once
#include "IRenderPass.h"
#include <vector>
#include <memory>

struct RenderContext;
struct FrameResource;

class RenderPipeline
{
public:
    void AddPass(std::unique_ptr<IRenderPass> pass);
    void Execute(const RenderContext& ctx, FrameResource* fr);

private:
    std::vector<std::unique_ptr<IRenderPass>> mPasses;
};
