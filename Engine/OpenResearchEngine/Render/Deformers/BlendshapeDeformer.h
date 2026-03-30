#pragma once
#include "IDeformer.h"
#include "../Resources/RenderMeshAsset.h"
#include "../../D3D12/D3DUtil.h"
#include <wrl/client.h>
#include <memory>

#include "../Resources/Blendshapes.h"

class BlendshapeDeformer : public IDeformer {
public:
    BlendshapeDeformer(
        ID3D12Device*               device,
        ID3D12GraphicsCommandList*  cmdList,
        RenderMeshAsset*            asset,
        UINT                        cbIndex,
        std::shared_ptr<BlendshapeController> controller,
        ID3D12RootSignature*        rootSig,
        ID3D12PipelineState*        pso
    );

    void Execute(ID3D12GraphicsCommandList*, const DeformContext&) override;
    DeformerType Type() const override { return DeformerType::Blendshape; }

    BlendshapeController* Controller() const { return mController.get(); }
    UINT CBIndex() const { return mCBIndex; }

    Microsoft::WRL::ComPtr<ID3D12Resource> BlendedVertexBufferGPU;

private:
    std::shared_ptr<BlendshapeController> mController;
    UINT                 mCBIndex;
    ID3D12RootSignature* mRootSig;
    ID3D12PipelineState* mPSO;
};
