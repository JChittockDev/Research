#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class SSS;
class RadianceResources;
class LightingPassResource;

class LightingPass : public IRenderPass
{
public:
    LightingPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBufferPassResource*             gBuffer,
        SSS*                 sss,
        RadianceResources*   radianceResources,
        LightingPassResource*            lighting);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Lighting"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    SSS*                 mSss;
    RadianceResources*   mRadianceResources;
    LightingPassResource*            mLighting;
};
