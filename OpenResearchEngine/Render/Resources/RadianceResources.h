#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
#include "../Resources/Radiance.h"
 
 
class RadianceResources
{
public:

    RadianceResources(ID3D12Device* device);
    RadianceResources(const RadianceResources& rhs) = delete;
    RadianceResources& operator=(const RadianceResources& rhs) = delete;
    ~RadianceResources() = default;

    void BuildDescriptors(const int numLights, UINT width, UINT height, CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv,
                                            CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuRtv, UINT srvDescriptorSize, UINT rtvDescriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuSrv() const { return startCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return startGpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuDsv() const { return startCpuRtv; }

    std::vector<std::unique_ptr<Radiance>> radianceMaps;

private:
	ID3D12Device* md3dDevice;

    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE startGpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuRtv;
};