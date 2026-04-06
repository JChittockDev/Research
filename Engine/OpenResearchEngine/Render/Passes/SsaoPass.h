#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class SsaoPassResource;

class SsaoPass : public IRenderPass
{
public:
    SsaoPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
             GBufferPassResource* gBuffer, SsaoPassResource* ssao);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSAO"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    SsaoPassResource*                mSsao;
};
