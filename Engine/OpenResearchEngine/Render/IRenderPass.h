#pragma once

struct RenderContext;
struct FrameResource;

class IRenderPass
{
public:
    virtual ~IRenderPass() = default;
    virtual void        Execute(const RenderContext& ctx, FrameResource* fr) = 0;
    virtual const char* Name() const = 0;
};
