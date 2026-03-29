#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBuffer;
class SSS;
class RadianceResources;
class Lighting;

class LightingPass : public IRenderPass
{
public:
    LightingPass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBuffer*             gBuffer,
        SSS*                 sss,
        RadianceResources*   radianceResources,
        Lighting*            lighting);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Lighting"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBuffer*             mGBuffer;
    SSS*                 mSss;
    RadianceResources*   mRadianceResources;
    Lighting*            mLighting;
};
