#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class SSS;

class SssBlurPass : public IRenderPass
{
public:
    SssBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, SSS* sss);
    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "SssBlur"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    SSS*                 mSss;
};
