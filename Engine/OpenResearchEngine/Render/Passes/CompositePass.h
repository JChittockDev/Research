#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class LightingPassInfo;
class Ssgi;

class CompositePass : public IRenderPass
{
public:
    CompositePass(
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        LightingPassInfo*            lighting,
        Ssgi*                ssgi);

    void        Execute(const RenderContext& ctx, FrameResource* fr) override;
    const char* Name() const override { return "Composite"; }

private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    LightingPassInfo*            mLighting;
    Ssgi*                mSsgi;
};
