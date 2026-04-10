#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class SssPassResource;
class RadianceResources;

class SssPass : public IRenderPass
{
public:
    SssPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
            GBufferPassResource* gBuffer, SssPassResource* sss, RadianceResources* radianceResources);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSS"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    SssPassResource*                 mSss;
    RadianceResources*   mRadianceResources;
};
