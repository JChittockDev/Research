#pragma once
#include "IRenderPass.h"
#include <vector>
#include <memory>

struct RenderContext;
struct RenderPassConstantBuffers;

class RenderPipeline
{
public:
    void AddPass(std::unique_ptr<IRenderPass> pass);
    void Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr);

private:
    std::vector<std::unique_ptr<IRenderPass>> mPasses;
};
