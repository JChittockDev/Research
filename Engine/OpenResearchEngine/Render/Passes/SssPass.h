#pragma once
#include "../IRenderPass.h"
#include <d3d12.h>

class GBufferPassResource;
class SssPassResource;
class RenderPassResourceArray;

class SssPass : public IRenderPass
{
public:
    SssPass(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso,
            GBufferPassResource* gBuffer, SssPassResource* sss, RenderPassResourceArray* radianceResources);
    void        Execute(const RenderContext& ctx, RenderPassConstantBuffers* fr) override;
    const char* Name() const override { return "SSS"; }
private:
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPso;
    GBufferPassResource*             mGBuffer;
    SssPassResource*                 mSss;
    RenderPassResourceArray*   mRadianceResources;
};
