#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBuffer;
class SSS;
class RadianceResources;

class SssPass : public IRenderPass
{
public:
    SssPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
            GBuffer* gBuffer, SSS* sss, RadianceResources* radianceResources);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSS"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBuffer*             mGBuffer;
    SSS*                 mSss;
    RadianceResources*   mRadianceResources;
};
