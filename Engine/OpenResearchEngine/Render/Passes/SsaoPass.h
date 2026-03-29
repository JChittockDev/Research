#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBuffer;
class Ssao;

class SsaoPass : public IRenderPass
{
public:
    SsaoPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
             GBuffer* gBuffer, Ssao* ssao);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SSAO"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBuffer*             mGBuffer;
    Ssao*                mSsao;
};
