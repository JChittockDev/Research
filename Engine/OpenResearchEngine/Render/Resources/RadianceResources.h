#pragma once

#include "../Resources/RadiancePassResource.h"
 
 
class RadianceResources
{
public:

    RadianceResources(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    RadianceResources(const RadianceResources& rhs) = delete;
    RadianceResources& operator=(const RadianceResources& rhs) = delete;
    ~RadianceResources() = default;

    void BuildDescriptors(const int numLights, UINT width, UINT height, CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv,
                                            CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuRtv, UINT srvDescriptorSize, UINT rtvDescriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuSrv() const { return startCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return startGpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuDsv() const { return startCpuRtv; }

    std::vector<std::unique_ptr<RadiancePassResource>> radianceMaps;

private:
	ID3D12Device* md3dDevice;
    ID3D12GraphicsCommandList* commandList;

    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE startGpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuRtv;
};