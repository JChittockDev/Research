#include "RenderTextures.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void RenderTextures::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv, UINT cbvSrvUavDescriptorSize, const std::map<std::string, std::string>& textureData, const std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>>& textures)
{
    startCpuSrv = hCpuSrv;
    startGpuSrv = hGpuSrv;

    std::vector<ComPtr<ID3D12Resource>> tex2DList;
    for (const auto& texture : textureData)
    {
        auto texResource = textures.at(texture.first)->first.Resource;
        assert(texResource != nullptr);
        tex2DList.push_back(texResource);
    }

    for (UINT i = 0; i < (UINT)tex2DList.size(); ++i)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Format = tex2DList[i]->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
        md3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hCpuSrv);

        cpuSrvHandles.push_back(hCpuSrv);
        gpuSrvHandles.push_back(hGpuSrv);

        hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);
        hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);
    }

    hCpuSrv = hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);
    hGpuSrv = hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);
}

RenderTextures::RenderTextures(ID3D12Device* device)
{
    md3dDevice = device;
}