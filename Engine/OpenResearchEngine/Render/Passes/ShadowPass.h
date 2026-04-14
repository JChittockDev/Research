#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class RenderPassResourceArray;
class RenderTextures;

class ShadowPass : public IRenderPass
{
public:
    ShadowPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        RenderPassResourceArray*     shadowResources,
        RenderTextures*      renderTextures);

    void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) override;
    const char* Name() const override { return "Shadow"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    RenderPassResourceArray*     mShadowResources;
    RenderTextures*      mRenderTextures;
};
