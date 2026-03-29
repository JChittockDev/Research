#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBuffer;
class ShadowResources;
class RadianceResources;

class RadiancePass : public IRenderPass
{
public:
    RadiancePass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBuffer*             gBuffer,
        ShadowResources*     shadowResources,
        RadianceResources*   radianceResources);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Radiance"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBuffer*             mGBuffer;
    ShadowResources*     mShadowResources;
    RadianceResources*   mRadianceResources;
};
