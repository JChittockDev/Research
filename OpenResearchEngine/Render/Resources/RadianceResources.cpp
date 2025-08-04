#include "RadianceResources.h"
#include <DirectXPackedVector.h>

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
        std::unique_ptr<Radiance> radianceMap = std::make_unique<Radiance>(md3dDevice, width, height);
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

RadianceResources::RadianceResources(ID3D12Device* device)
{
    md3dDevice = device;
}