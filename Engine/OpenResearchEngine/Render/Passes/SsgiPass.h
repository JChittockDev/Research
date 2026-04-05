#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class LightingPassResource;
class GBufferPassResource;
class Ssgi;

class SsgiPass : public IRenderPass
{
public:
    SsgiPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
             LightingPassResource* lighting, GBufferPassResource* gBuffer, Ssgi* ssgi);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSGI"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    LightingPassResource*            mLighting;
    GBufferPassResource*             mGBuffer;
    Ssgi*                mSsgi;
};
