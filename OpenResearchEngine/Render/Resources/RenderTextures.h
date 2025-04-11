#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../Resources/FrameResource.h"
 
 
class RenderTextures
{
public:

    RenderTextures(ID3D12Device* device);
    RenderTextures(const RenderTextures& rhs) = delete;
    RenderTextures& operator=(const RenderTextures& rhs) = delete;
    ~RenderTextures() = default;

    void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE& hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE& hGpuSrv, UINT cbvSrvUavDescriptorSize, const std::map<std::string, std::string>& textureData,
                                                            const std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>>& textures);

    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> GetCpuSrvHandles() { return cpuSrvHandles; }
    std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> GetGpuSrvHandles() { return gpuSrvHandles; }

    int GetNumCpuSrvHandles() { return cpuSrvHandles.size(); }
    int GetNumGpuSrvHandles() { return gpuSrvHandles.size(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuSrv() const { return startCpuSrv; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return startGpuSrv; }

private:
	ID3D12Device* md3dDevice;

    CD3DX12_CPU_DESCRIPTOR_HANDLE startCpuSrv;
    CD3DX12_GPU_DESCRIPTOR_HANDLE startGpuSrv;
    
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuSrvHandles;
    std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuSrvHandles;
};