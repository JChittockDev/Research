#include "AnimationPass.h"
#include "../RenderContext.h"
#include "../Resources/FrameResource.h"

AnimationPass::AnimationPass(const AnimationPassResources& res) : mRes(res) {}

void AnimationPass::Execute(const RenderContext& ctx, FrameResource* fr)
{
    // GPU skinning/blendshape dispatch — pending update to DeformationGraph API
}

void AnimationPass::ComputeBlendshapes(ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
void AnimationPass::ComputeSkinning   (ID3D12GraphicsCommandList*, const std::shared_ptr<RenderItem>&, FrameResource*) {}
