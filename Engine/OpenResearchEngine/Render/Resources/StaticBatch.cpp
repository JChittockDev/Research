#include "StaticBatch.h"
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/D3Dx12.h"

StaticBatch::StaticBatch(RenderMeshAsset* asset, const std::string& subsetName, Material* mat)
    : mAsset(asset), mSubsetName(subsetName), mMat(mat) {}

void StaticBatch::AddInstance(const DirectX::XMFLOAT4X4& world) {
    mWorldMatrices.push_back(world);
}

void StaticBatch::UploadInstanceBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    if (mWorldMatrices.empty()) return;
    UINT byteSize = (UINT)(mWorldMatrices.size() * sizeof(DirectX::XMFLOAT4X4));
    mInstanceBufferGPU = d3dUtil::CreateDefaultBuffer(
        device, cmdList, mWorldMatrices.data(), byteSize, mInstanceBufferUploader);
}

void StaticBatch::Draw(ID3D12GraphicsCommandList* cmdList) const
{
    if (!mInstanceBufferGPU || mWorldMatrices.empty()) return;
    const auto& args = mAsset->DrawArgs.at(mSubsetName);
    auto vbv = mAsset->VertexBufferView();
    auto ibv = mAsset->IndexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetIndexBuffer(&ibv);
    cmdList->DrawIndexedInstanced(
        args.IndexCount, (UINT)mWorldMatrices.size(),
        args.IndexStart, args.VertexStart, 0);
}
