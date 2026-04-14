#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class RenderPassResourceArray;

class RadiancePass : public IRenderPass
{
public:
    RadiancePass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        GBufferPassResource*             gBuffer,
        RenderPassResourceArray*     shadowResources,
        RenderPassResourceArray*   radianceResources);
    void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) override;
    const char* Name() const override { return "Radiance"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    RenderPassResourceArray*     mShadowResources;
    RenderPassResourceArray*   mRadianceResources;
};
