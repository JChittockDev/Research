#include "RadianceResources.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void RadianceResources::BuildDescriptors(const int numLights, UINT width, UINT height, CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv,
                                                                CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuRtv, UINT srvDescriptorSize, UINT rtvDescriptorSize)
{
    startCpuRtv = hCpuRtv;
    startCpuSrv = hCpuSrv;
    startGpuSrv = hGpuSrv;

    for (int i = 0; i < numLights; i++)
    {
        std::unique_ptr<RadiancePassResource> radianceMap = std::make_unique<RadiancePassResource>(md3dDevice, commandList, width, height, 2);
        radianceMaps.push_back(std::move(radianceMap));
    }

    // Do diffuse first so they are sequential in the descriptor heap
    for (int i = 0; i < numLights; i++)
    {
        radianceMaps[i]->BuildDiffuseDescriptors(hCpuRtv, hCpuSrv, hGpuSrv, rtvDescriptorSize, srvDescriptorSize);
    }

    // Do specular second so they are sequential in the descriptor heap
    for (int i = 0; i < numLights; i++)
    {
        radianceMaps[i]->BuildSpecularDescriptors(hCpuRtv, hCpuSrv, hGpuSrv, rtvDescriptorSize, srvDescriptorSize);
    }
}

RadianceResources::RadianceResources(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    md3dDevice = device;
    commandList = cmdList;
}