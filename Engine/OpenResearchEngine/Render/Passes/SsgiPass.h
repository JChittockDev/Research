#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class Lighting;
class GBuffer;
class Ssgi;

class SsgiPass : public IRenderPass
{
public:
    SsgiPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
             Lighting* lighting, GBuffer* gBuffer, Ssgi* ssgi);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSGI"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    Lighting*            mLighting;
    GBuffer*             mGBuffer;
    Ssgi*                mSsgi;
};
