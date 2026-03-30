#pragma once
#include "RenderMeshAsset.h"
#include "../../Common/Structures.h"
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <wrl/client.h>

class StaticBatch {
public:
    StaticBatch(RenderMeshAsset* asset, const std::string& subsetName, Material* mat);

    void AddInstance(const DirectX::XMFLOAT4X4& world);
    void UploadInstanceBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void Draw(ID3D12GraphicsCommandList* cmdList) const;

    RenderMeshAsset* Asset()   const { return mAsset; }
    Material*        GetMat()  const { return mMat; }
    UINT             ObjCBIndex = 0; // Shared object CB index for this batch

private:
    RenderMeshAsset*  mAsset;
    std::string       mSubsetName;
    Material*         mMat;
    std::vector<DirectX::XMFLOAT4X4> mWorldMatrices;
    Microsoft::WRL::ComPtr<ID3D12Resource> mInstanceBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> mInstanceBufferUploader;
};
