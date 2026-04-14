#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>
#include <memory>

struct RenderItem;

struct AnimationPassResources
{
    ID3D12RootSignature* blend   = nullptr;
    ID3D12RootSignature* skinned = nullptr;
    ID3D12PipelineState* psoBlend   = nullptr;
    ID3D12PipelineState* psoSkinned = nullptr;
};

class AnimationPass : public IRenderPass
{
public:
    explicit AnimationPass(const AnimationPassResources& res);
    void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) override;
    const char* Name() const override { return "Animation"; }

private:
    void ComputeBlendshapes(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, RenderPassConstantBuffers*);
    void ComputeSkinning   (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, RenderPassConstantBuffers*);

    AnimationPassResources mRes;
};
