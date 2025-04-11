#include "ShadowResources.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

void ShadowResources::BuildDescriptors(const int numLights, CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv, 
                                                                CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuDsv, UINT srvDescriptorSize, UINT dsvDescriptorSize)
{

    startCpuSrv = hCpuSrv;
    startGpuSrv = hGpuSrv;
    startCpuDsv = hCpuDsv;

    for (int i = 0; i < numLights; i++)
    {
        shadowMaps.push_back(std::make_unique<ShadowMap>(md3dDevice, 2048, 2048));
        shadowMaps[i]->BuildDescriptors(hCpuSrv, hGpuSrv, hCpuDsv);

        cpuSrvHandles.push_back(hCpuSrv);
        gpuSrvHandles.push_back(hGpuSrv);
        cpuDsvHandles.push_back(hCpuDsv);
        
        hCpuSrv.Offset(1, srvDescriptorSize);
        hGpuSrv.Offset(1, srvDescriptorSize);
        hCpuDsv.Offset(1, dsvDescriptorSize);
    }

    hCpuSrv = hCpuSrv.Offset(1, srvDescriptorSize);
    hGpuSrv = hGpuSrv.Offset(1, srvDescriptorSize);
    hCpuDsv = hCpuDsv.Offset(1, dsvDescriptorSize);

}

ShadowResources::ShadowResources(ID3D12Device* device)
{
    md3dDevice = device;
}