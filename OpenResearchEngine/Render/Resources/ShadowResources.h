#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
#include "../Resources/ShadowMap.h"
 
 
class ShadowResources
{
public:

    ShadowResources(ID3D12Device* device);
    ShadowResources(const ShadowResources& rhs) = delete;
    ShadowResources& operator=(const ShadowResources& rhs) = delete;
    ~ShadowResources() = default;

    void BuildDescriptors(const int numLights, CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv,
                                            CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuDsv, UINT srvDescriptorSize, UINT dsvDescriptorSize);

    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> GetCpuSrvHandles() { return cpuSrvHandles; }
    std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> GetGpuSrvHandles() { return gpuSrvHandles; }
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> GetCpuDsvHandles() { return cpuDsvHandles; }

    size_t GetNumCpuSrvHandles() { return cpuSrvHandles.size(); }
    size_t GetNumGpuSrvHandles() { return gpuSrvHandles.size(); }
    size_t GetNumCpuDsvHandles() { return cpuDsvHandles.size(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuSrv() const { return startCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return startGpuSrv; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuDsv() const { return startCpuDsv; }

    std::vector<std::unique_ptr<ShadowMap>> shadowMaps;

private:
	ID3D12Device* md3dDevice;

    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE startGpuSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuDsv;
    
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuSrvHandles;
    std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuSrvHandles;
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuDsvHandles;
};