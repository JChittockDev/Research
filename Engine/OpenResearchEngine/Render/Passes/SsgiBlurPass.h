#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBuffer;
class Ssgi;

class SsgiBlurPass : public IRenderPass
{
public:
    SsgiBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
                 GBuffer* gBuffer, Ssgi* ssgi);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SsgiBlur"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBuffer*             mGBuffer;
    Ssgi*                mSsgi;
};
