#pragma once
#include "../IRenderPass.h"
#include <wrl/client.h>
#include <d3d12.h>

class GBufferPassResource;
class RenderTextures;

class GBufferPass : public IRenderPass
{
public:
    GBufferPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBufferPassResource*             gBuffer,
        RenderTextures*      renderTextures);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "GBuffer"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    RenderTextures*      mRenderTextures;
};
