#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class SssPassResource;

class SssBlurPass : public IRenderPass
{
public:
    SssBlurPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, SssPassResource* sss);
    void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) override;
    const char* Name() const override { return "SssBlur"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    SssPassResource*                 mSss;
};
