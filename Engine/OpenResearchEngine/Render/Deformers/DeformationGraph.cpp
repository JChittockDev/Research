#include "DeformationGraph.h"
#include "BlendshapeDeformer.h"
#include "SkinDeformer.h"
#include "PhysicsDeformer.h"

void DeformationGraph::AddDeformer(std::unique_ptr<IDeformer> d) {
    mDeformers.push_back(std::move(d));
}

void DeformationGraph::Execute(ID3D12GraphicsCommandList* cmd, const DeformContext& baseCtx)
{
    DeformContext ctx = baseCtx;

    // Pre-collect BlendedVertexBuffer so SkinDeformer can read + reset it
    for (auto& d : mDeformers) {
        if (d->Type() == DeformerType::Blendshape)
            ctx.BlendedVertexBuffer =
                static_cast<BlendshapeDeformer*>(d.get())->BlendedVertexBufferGPU.Get();
    }

    for (auto& d : mDeformers) {
        d->Execute(cmd, ctx);
        // After SkinDeformer, expose its output for PhysicsDeformer
        if (d->Type() == DeformerType::Skin)
            ctx.SkinnedVertexBuffer =
                static_cast<SkinDeformer*>(d.get())->SkinnedVertexBufferGPU.Get();
    }
}

ID3D12Resource* DeformationGraph::FinalVertexBuffer() const
{
    // Walk in reverse — return the last meaningful output
    for (auto it = mDeformers.rbegin(); it != mDeformers.rend(); ++it) {
        if ((*it)->Type() == DeformerType::Physics)
            return static_cast<PhysicsDeformer*>(it->get())->VertexNormalBufferGPU.Get();
        if ((*it)->Type() == DeformerType::Skin)
            return static_cast<SkinDeformer*>(it->get())->SkinnedVertexBufferGPU.Get();
    }
    return nullptr;
}
