#pragma once

struct RenderContext;
struct RenderPassConstantBuffers;

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;
    virtual void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) = 0;
    virtual void        Update() {};
    virtual const char* Name() const = 0;
};
