#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class ShadowResources;
class RenderTextures;

class ShadowPass : public IRenderPass
{
public:
    ShadowPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        ShadowResources*     shadowResources,
        RenderTextures*      renderTextures);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Shadow"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    ShadowResources*     mShadowResources;
    RenderTextures*      mRenderTextures;
};
