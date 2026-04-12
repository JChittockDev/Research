#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class SssPassResource;
class RenderPassResourceArray;
class LightingPassResource;

class LightingPass : public IRenderPass
{
public:
    LightingPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBufferPassResource*             gBuffer,
        SssPassResource*                 sss,
        RenderPassResourceArray*   radianceResources,
        LightingPassResource*            lighting);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Lighting"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    SssPassResource*                 mSss;
    RenderPassResourceArray*   mRadianceResources;
    LightingPassResource*            mLighting;
};
