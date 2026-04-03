# Descriptor Heap Refactor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace brittle cursor-threading in `SetRenderPassResources` with a bump-allocator `DescriptorHeap` and `DescriptorAllocation` value type, eliminating 28+ individual handle members and all implicit ordering dependencies.

**Architecture:** Two new types: `DescriptorAllocation` (value type — base handle + stride + count → index-based access) and `DescriptorHeap` (bump allocator over a single `ID3D12DescriptorHeap`). Each resource class allocates once at startup, stores the allocation, and computes sub-handles by index. `SetRenderPassResources` becomes a flat, order-independent list of `BuildDescriptors` calls.

**Tech Stack:** C++17, DirectX 12, MSVC Visual Studio 2022 x64. No test framework — verification is build + runtime. Build command: `powershell.exe -Command "& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' 'D:\Programming\Research\Engine\OpenResearchEngine\OpenResearchEngine.vcxproj' /p:Configuration=Debug /p:Platform=x64 2>&1 | Select-String -Pattern 'error|warning|Build succeeded' | Out-String"`

---

## File Map

| File | Change |
|------|--------|
| `D3D12/DescriptorAllocation.h` | **New** — header-only value type |
| `D3D12/DescriptorHeap.h` | **New** — allocator header |
| `D3D12/DescriptorHeap.cpp` | **New** — allocator implementation |
| `D3D12/D3DApp.h` | Remove `renderPassRtvHeap` / `renderPassSrvHeap` |
| `EngineApp.h` | Add 3 `DescriptorHeap` members; remove `mSrvDescriptorHeap`; include new header |
| `Build/PopulateDescriptorHeaps.cpp` | Remove old heap creation; reduce `mDsvHeap` to 1; call `Create()` on 3 new heaps |
| `Render/Manager/Render.cpp` | `renderPassSrvHeap.Get()` → `mPassSrvHeap.Heap()` |
| `Render/Resources/GBuffer.h/.cpp` | Replace 14 CD3DX12 handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/Ssao.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/Ssgi.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/SSS.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/Lighting.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/Composite.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update all methods |
| `Render/Resources/ShadowResources.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update `BuildDescriptors` |
| `Render/Resources/RadianceResources.h/.cpp` | Replace handle members with 2 `DescriptorAllocation`; update `BuildDescriptors` |
| `Render/Resources/RenderTextures.h/.cpp` | Replace handle members with 1 `DescriptorAllocation`; update `BuildDescriptors` |
| `Build/SetRenderPassResources.cpp` | Remove cursor threading; pass heap refs to each `BuildDescriptors` |
| `OpenResearchEngine.vcxproj` | Add `D3D12\DescriptorHeap.cpp` ClCompile entry |

> **Build note:** Tasks 1–3 produce a clean build after each task. Tasks 4–14 are an atomic compilation unit — the project will not build again until **all** of Tasks 4–14 are complete. The single build verification step is in Task 14.

---

## Task 1: Create feature branch

**Files:** none

- [ ] **Step 1: Create and switch to the feature branch**

```bash
cd "D:\Programming\Research\Engine\OpenResearchEngine"
git checkout -b feature/descriptor-heap-refactor
```

Expected: `Switched to a new branch 'feature/descriptor-heap-refactor'`

- [ ] **Step 2: Commit**

```bash
git commit --allow-empty -m "chore: start descriptor-heap-refactor branch"
```

---

## Task 2: Create DescriptorAllocation.h

**Files:**
- Create: `D3D12/DescriptorAllocation.h`

The `DescriptorAllocation` is a value type representing a contiguous range of descriptors within a heap. `Cpu(i)` and `Gpu(i)` compute sub-handles by index. `Gpu()` asserts the heap was shader-visible (detected by checking `gpuStart.ptr != 0`).

- [ ] **Step 1: Create the header**

Create `D3D12/DescriptorAllocation.h` with this content:

```cpp
#pragma once
#include "D3DUtil.h"

struct DescriptorAllocation
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = {};  // only valid for shader-visible heaps
    UINT count  = 0;
    UINT stride = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE Cpu(UINT i) const
    {
        assert(i < count);
        return D3D12_CPU_DESCRIPTOR_HANDLE{ cpuStart.ptr + (UINT64)i * stride };
    }

    D3D12_GPU_DESCRIPTOR_HANDLE Gpu(UINT i) const
    {
        assert(i < count);
        assert(gpuStart.ptr != 0 && "Gpu() called on a non-shader-visible allocation");
        return D3D12_GPU_DESCRIPTOR_HANDLE{ gpuStart.ptr + (UINT64)i * stride };
    }

    bool IsValid() const { return count > 0; }
};
```

- [ ] **Step 2: Build and verify no errors**

Run the build command. Expected: Build succeeds (header-only addition, nothing broken yet).

- [ ] **Step 3: Commit**

```bash
git add D3D12/DescriptorAllocation.h
git commit -m "feat: add DescriptorAllocation value type"
```

---

## Task 3: Create DescriptorHeap class + add to vcxproj

**Files:**
- Create: `D3D12/DescriptorHeap.h`
- Create: `D3D12/DescriptorHeap.cpp`
- Modify: `OpenResearchEngine.vcxproj`

A bump allocator over a single `ID3D12DescriptorHeap`. No de-allocation. Overflow is caught immediately in debug builds via `assert`. Not a singleton — instances are owned by `EngineApp`.

- [ ] **Step 1: Create DescriptorHeap.h**

```cpp
#pragma once
#include "D3DUtil.h"
#include "DescriptorAllocation.h"

class DescriptorHeap
{
public:
    void Create(ID3D12Device* device,
                D3D12_DESCRIPTOR_HEAP_TYPE type,
                UINT capacity,
                bool shaderVisible);

    DescriptorAllocation Allocate(UINT count);

    ID3D12DescriptorHeap* Heap()     const { return mHeap.Get(); }
    UINT                  Stride()   const { return mStride; }
    UINT                  Used()     const { return mNextFree; }
    UINT                  Capacity() const { return mCapacity; }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
    UINT mStride    = 0;
    UINT mNextFree  = 0;
    UINT mCapacity  = 0;
};
```

- [ ] **Step 2: Create DescriptorHeap.cpp**

```cpp
#include "DescriptorHeap.h"

void DescriptorHeap::Create(ID3D12Device* device,
                             D3D12_DESCRIPTOR_HEAP_TYPE type,
                             UINT capacity,
                             bool shaderVisible)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type           = type;
    desc.NumDescriptors = capacity;
    desc.Flags          = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                        : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask       = 0;
    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mHeap.GetAddressOf())));

    mStride    = device->GetDescriptorHandleIncrementSize(type);
    mNextFree  = 0;
    mCapacity  = capacity;
}

DescriptorAllocation DescriptorHeap::Allocate(UINT count)
{
    assert(mNextFree + count <= mCapacity && "DescriptorHeap overflow");

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += (UINT64)mNextFree * mStride;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = {};
    D3D12_DESCRIPTOR_HEAP_DESC desc = mHeap->GetDesc();
    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        gpu = mHeap->GetGPUDescriptorHandleForHeapStart();
        gpu.ptr += (UINT64)mNextFree * mStride;
    }

    mNextFree += count;

    DescriptorAllocation alloc;
    alloc.cpuStart = cpu;
    alloc.gpuStart = gpu;
    alloc.count    = count;
    alloc.stride   = mStride;
    return alloc;
}
```

- [ ] **Step 3: Add DescriptorHeap.cpp to vcxproj**

In `OpenResearchEngine.vcxproj`, find the existing `D3D12\D3DUtil.cpp` ClCompile entry and add a new entry immediately after it:

```xml
    <ClCompile Include="D3D12\DescriptorHeap.cpp" />
```

- [ ] **Step 4: Build and verify**

Run the build command. Expected: Build succeeds with 0 errors.

- [ ] **Step 5: Commit**

```bash
git add D3D12/DescriptorHeap.h D3D12/DescriptorHeap.cpp OpenResearchEngine.vcxproj
git commit -m "feat: add DescriptorHeap bump allocator"
```

---

## Task 4: Infrastructure swap — D3DApp, EngineApp, PopulateDescriptorHeaps, Render

**Files:**
- Modify: `D3D12/D3DApp.h`
- Modify: `EngineApp.h`
- Modify: `Build/PopulateDescriptorHeaps.cpp`
- Modify: `Render/Manager/Render.cpp`

> **Build note:** After this task the project will NOT build. That is expected. The build is recovered in Task 14 once all resource classes and `SetRenderPassResources.cpp` are also updated.

- [ ] **Step 1: Update D3DApp.h — remove renderPassRtvHeap / renderPassSrvHeap**

In `D3D12/D3DApp.h`, find and remove these two lines (around line 125–126):

```cpp
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderPassRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderPassSrvHeap;
```

- [ ] **Step 2: Update EngineApp.h — add heap members and include**

Add the include after the existing D3D12 includes (near the top, after `D3D12/D3DApp.h`):

```cpp
#include "D3D12/DescriptorHeap.h"
```

Find the private member section (around line 118) and replace:

```cpp
    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap  = nullptr;
    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap = nullptr;
```

with:

```cpp
    DescriptorHeap mPassRtvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_RTV, cap 64, not shader-visible
    DescriptorHeap mPassSrvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cap 128, shader-visible
    DescriptorHeap mPassDsvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_DSV, cap 16, not shader-visible
    ComPtr<ID3D12DescriptorHeap> imGuiSrvDescriptorHeap = nullptr;
```

- [ ] **Step 3: Update PopulateDescriptorHeaps.cpp**

Replace the entire file content with:

```cpp
#include "../EngineApp.h"

void EngineApp::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

    // Capacity 1: main depth buffer only. Shadow DSVs go into mPassDsvHeap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

    mPassRtvHeap.Create(md3dDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,         64,  false);
    mPassSrvHeap.Create(md3dDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
    mPassDsvHeap.Create(md3dDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV,         16,  false);

    D3D12_DESCRIPTOR_HEAP_DESC imguiDesc = {};
    imguiDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    imguiDesc.NumDescriptors = 1;
    imguiDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&imguiDesc, IID_PPV_ARGS(&imGuiSrvDescriptorHeap)));
}
```

- [ ] **Step 4: Update Render/Manager/Render.cpp — switch to mPassSrvHeap**

In `Render/Manager/Render.cpp`, find this line (around line 10):

```cpp
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderPassSrvHeap.Get() };
```

Replace with:

```cpp
    ID3D12DescriptorHeap* descriptorHeaps[] = { mPassSrvHeap.Heap() };
```

---

## Task 5: Update GBuffer.h/.cpp

**Files:**
- Modify: `Render/Resources/GBuffer.h`
- Modify: `Render/Resources/GBuffer.cpp`

GBuffer has 7 RTVs and 7 SRVs in this order:
- RTV/SRV[0] = Position, [1] = Normal, [2] = ViewNormal, [3] = AlbedoSpec, [4] = Reflection, [5] = MaterialId, [6] = Tangent

The 14 GPU RTV members (`mhPositionGpuRtv` etc.) are dead — RTV heaps are CPU-only. They are removed entirely.
The 14 CPU SRV members are also removed — RebuildDescriptors uses `mSrvAlloc.Cpu(i)` directly.

- [ ] **Step 1: Replace GBuffer.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class GBuffer
{
public:
    GBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    GBuffer(const GBuffer& rhs) = delete;
    GBuffer& operator=(const GBuffer& rhs) = delete;
    ~GBuffer() = default;

    UINT GBufferWidth()  const;
    UINT GBufferHeight() const;

    void BuildDescriptors(ID3D12Device* device, class DescriptorHeap& rtvHeap, class DescriptorHeap& srvHeap);
    void RebuildDescriptors();
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildResources();

    void CreateGBufferTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateGBufferRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateGBufferSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

    Microsoft::WRL::ComPtr<ID3D12Resource> GetPosition()    const { return mPosition; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetNormal()      const { return mNormal; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetViewNormal()  const { return mViewNormal; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAlbedoSpec()  const { return mAlbedoSpec; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetReflection()  const { return mReflection; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialId()  const { return mMaterialId; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetTangent()     const { return mTangent; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionCpuRtv()   const { return mRtvAlloc.Cpu(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalCpuRtv()     const { return mRtvAlloc.Cpu(1); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetViewNormalCpuRtv() const { return mRtvAlloc.Cpu(2); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoSpecCpuRtv() const { return mRtvAlloc.Cpu(3); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetReflectionCpuRtv() const { return mRtvAlloc.Cpu(4); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetMaterialIdCpuRtv() const { return mRtvAlloc.Cpu(5); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetTangentCpuRtv()    const { return mRtvAlloc.Cpu(6); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetPositionGpuSrv()   const { return mSrvAlloc.Gpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetNormalGpuSrv()     const { return mSrvAlloc.Gpu(1); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetViewNormalGpuSrv() const { return mSrvAlloc.Gpu(2); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoSpecGpuSrv() const { return mSrvAlloc.Gpu(3); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetReflectionGpuSrv() const { return mSrvAlloc.Gpu(4); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetMaterialIdGpuSrv() const { return mSrvAlloc.Gpu(5); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetTangentGpuSrv()    const { return mSrvAlloc.Gpu(6); }

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mPosition;
    Microsoft::WRL::ComPtr<ID3D12Resource> mNormal;
    Microsoft::WRL::ComPtr<ID3D12Resource> mViewNormal;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAlbedoSpec;
    Microsoft::WRL::ComPtr<ID3D12Resource> mReflection;
    Microsoft::WRL::ComPtr<ID3D12Resource> mMaterialId;
    Microsoft::WRL::ComPtr<ID3D12Resource> mTangent;

    DescriptorAllocation mRtvAlloc;  // 7 contiguous RTVs
    DescriptorAllocation mSrvAlloc;  // 7 contiguous SRVs

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};
```

- [ ] **Step 2: Update GBuffer.cpp — BuildDescriptors and RebuildDescriptors**

Add include at the top of GBuffer.cpp:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void GBuffer::BuildDescriptors(ID3D12Device* device, DescriptorHeap& rtvHeap, DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(7);
    mSrvAlloc = srvHeap.Allocate(7);
    RebuildDescriptors();
}
```

Replace `RebuildDescriptors`:
```cpp
void GBuffer::RebuildDescriptors()
{
    DXGI_FORMAT rtvFormats[7] = {
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Position
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R16G16B16A16_FLOAT, // ViewNormal
        DXGI_FORMAT_R8G8B8A8_UNORM,     // AlbedoSpec
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Reflection
        DXGI_FORMAT_R32G32B32A32_FLOAT, // MaterialId
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Tangent
    };
    Microsoft::WRL::ComPtr<ID3D12Resource>* textures[7] = {
        &mPosition, &mNormal, &mViewNormal, &mAlbedoSpec, &mReflection, &mMaterialId, &mTangent
    };
    for (int i = 0; i < 7; i++)
    {
        CreateGBufferRTV(rtvFormats[i], *textures[i], mRtvAlloc.Cpu(i));
        CreateGBufferSRV(rtvFormats[i], *textures[i], mSrvAlloc.Cpu(i));
    }
}
```

Replace `CreateGBufferRTV` and `CreateGBufferSRV` signatures (change from `CD3DX12_CPU_DESCRIPTOR_HANDLE&` to `D3D12_CPU_DESCRIPTOR_HANDLE`):

```cpp
void GBuffer::CreateGBufferRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void GBuffer::CreateGBufferSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = format;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    md3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);
}
```

---

## Task 6: Update Ssao.h/.cpp

**Files:**
- Modify: `Render/Resources/Ssao.h`
- Modify: `Render/Resources/Ssao.cpp`

Ssao has 3 RTVs and 5 SRVs:
- RTV[0]=Ambient, [1]=AmbientVertBlur, [2]=AmbientHorizBlur
- SRV[0]=Ambient, [1]=AmbientVertBlur, [2]=AmbientHorizBlur, [3]=RandomVector, [4]=Depth

Dead members removed: all GPU RTV handles (`mhAmbientGpuRtv` etc.) — RTV heaps are CPU-only.

- [ ] **Step 1: Replace Ssao.h**

```cpp
#ifndef SSAO_H
#define SSAO_H
#pragma once

#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class Ssao
{
public:
    Ssao(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Ssao(const Ssao& rhs) = delete;
    Ssao& operator=(const Ssao& rhs) = delete;
    ~Ssao() = default;

    UINT SsaoWidth()  const;
    UINT SsaoHeight() const;

    static const int MaxBlurRadius = 5;

    void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);

    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbient()              const { return mAmbient; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbientVerticalBlur()  const { return mAmbientVerticalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetAmbientHorizontalBlur() const { return mAmbientHorizontalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetRandomVector()         const { return mRandomVector; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientCpuRtv()              const { return mRtvAlloc.Cpu(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurCpuRtv()  const { return mRtvAlloc.Cpu(1); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurCpuRtv() const { return mRtvAlloc.Cpu(2); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientGpuSrv()              const { return mSrvAlloc.Gpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientVerticalBlurGpuSrv()  const { return mSrvAlloc.Gpu(1); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientHorizontalBlurGpuSrv() const { return mSrvAlloc.Gpu(2); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuSrv()         const { return mSrvAlloc.Gpu(3); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv()                const { return mSrvAlloc.Gpu(4); }

    void BuildDescriptors(ID3D12Device* device,
                          Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                          class DescriptorHeap& rtvHeap,
                          class DescriptorHeap& srvHeap);
    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);
    void BuildOffsetVectors();
    std::vector<float> CalcGaussWeights(float sigma);

    void CreateSsaoTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSsaoRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateSsaoSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);
    void BuildResources();

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVector;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorUploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbient;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbientVerticalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mAmbientHorizontalBlur;

    DescriptorAllocation mRtvAlloc;  // 3 RTVs: Ambient(0), VertBlur(1), HorizBlur(2)
    DescriptorAllocation mSrvAlloc;  // 5 SRVs: Ambient(0), VertBlur(1), HorizBlur(2), RandomVec(3), Depth(4)

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    DirectX::XMFLOAT4 mOffsets[14];

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};

#endif // SSAO_H
```

- [ ] **Step 2: Update Ssao.cpp — BuildDescriptors and RebuildDescriptors**

Add include at the top of Ssao.cpp:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void Ssao::BuildDescriptors(ID3D12Device* device,
                             Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                             DescriptorHeap& rtvHeap,
                             DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(3);
    mSrvAlloc = srvHeap.Allocate(5);
    RebuildDescriptors(depthStencilBuffer);
}
```

Replace `RebuildDescriptors`:
```cpp
void Ssao::RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbient,              mRtvAlloc.Cpu(0));
    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbientVerticalBlur,  mRtvAlloc.Cpu(1));
    CreateSsaoRTV(DXGI_FORMAT_R16_UNORM, mAmbientHorizontalBlur, mRtvAlloc.Cpu(2));

    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM,              mAmbient,              mSrvAlloc.Cpu(0));
    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM,              mAmbientVerticalBlur,  mSrvAlloc.Cpu(1));
    CreateSsaoSRV(DXGI_FORMAT_R16_UNORM,              mAmbientHorizontalBlur, mSrvAlloc.Cpu(2));
    CreateSsaoSRV(DXGI_FORMAT_R8G8B8A8_UNORM,         mRandomVector,          mSrvAlloc.Cpu(3));
    CreateSsaoSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS,  depthStencilBuffer,     mSrvAlloc.Cpu(4));
}
```

Update `CreateSsaoRTV` and `CreateSsaoSRV` to take `D3D12_CPU_DESCRIPTOR_HANDLE` by value (not `CD3DX12_CPU_DESCRIPTOR_HANDLE&`):
```cpp
void Ssao::CreateSsaoRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
}

void Ssao::CreateSsaoSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = format;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    md3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);
}
```

---

## Task 7: Update Ssgi.h/.cpp

**Files:**
- Modify: `Render/Resources/Ssgi.h`
- Modify: `Render/Resources/Ssgi.cpp`

Ssgi has 3 RTVs and 5 SRVs (same layout as Ssao):
- RTV[0]=GI, [1]=GIVertBlur, [2]=GIHorizBlur
- SRV[0]=GI, [1]=GIVertBlur, [2]=GIHorizBlur, [3]=RandomVector, [4]=Depth

- [ ] **Step 1: Replace Ssgi.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class Ssgi
{
public:
    Ssgi(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Ssgi(const Ssgi& rhs) = delete;
    Ssgi& operator=(const Ssgi& rhs) = delete;
    ~Ssgi() = default;

    UINT SsgiWidth()  const;
    UINT SsgiHeight() const;

    static const int MaxBlurRadius = 5;

    void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);

    Microsoft::WRL::ComPtr<ID3D12Resource> GetGI()              const { return mGI; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetGIVerticalBlur()  const { return mGIVerticalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetGIHorizontalBlur() const { return mGIHorizontalBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetRandomVector()    const { return mRandomVector; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetGICpuRtv()              const { return mRtvAlloc.Cpu(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIVerticalBlurCpuRtv()  const { return mRtvAlloc.Cpu(1); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurCpuRtv() const { return mRtvAlloc.Cpu(2); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGIGpuSrv()              const { return mSrvAlloc.Gpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIVerticalBlurGpuSrv()  const { return mSrvAlloc.Gpu(1); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGIHorizontalBlurGpuSrv() const { return mSrvAlloc.Gpu(2); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuSrv()    const { return mSrvAlloc.Gpu(3); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv()           const { return mSrvAlloc.Gpu(4); }

    void BuildDescriptors(ID3D12Device* device,
                          Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                          class DescriptorHeap& rtvHeap,
                          class DescriptorHeap& srvHeap);
    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);
    void BuildOffsetVectors();
    std::vector<float> CalcGaussWeights(float sigma);

    void CreateSsgiTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSsgiRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateSsgiSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);
    void BuildResources();

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mGI;
    Microsoft::WRL::ComPtr<ID3D12Resource> mGIVerticalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mGIHorizontalBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVector;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorUploadBuffer;

    DescriptorAllocation mRtvAlloc;  // 3 RTVs: GI(0), VertBlur(1), HorizBlur(2)
    DescriptorAllocation mSrvAlloc;  // 5 SRVs: GI(0), VertBlur(1), HorizBlur(2), RandomVec(3), Depth(4)

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    DirectX::XMFLOAT4 mOffsets[14];

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};
```

- [ ] **Step 2: Update Ssgi.cpp — BuildDescriptors, RebuildDescriptors, helper signatures**

Add include at the top of Ssgi.cpp:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void Ssgi::BuildDescriptors(ID3D12Device* device,
                             Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                             DescriptorHeap& rtvHeap,
                             DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(3);
    mSrvAlloc = srvHeap.Allocate(5);
    RebuildDescriptors(depthStencilBuffer);
}
```

Replace `RebuildDescriptors`:
```cpp
void Ssgi::RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CreateSsgiRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mGI,              mRtvAlloc.Cpu(0));
    CreateSsgiRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mGIVerticalBlur,  mRtvAlloc.Cpu(1));
    CreateSsgiRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mGIHorizontalBlur, mRtvAlloc.Cpu(2));

    CreateSsgiSRV(DXGI_FORMAT_R8G8B8A8_UNORM,            mGI,              mSrvAlloc.Cpu(0));
    CreateSsgiSRV(DXGI_FORMAT_R8G8B8A8_UNORM,            mGIVerticalBlur,  mSrvAlloc.Cpu(1));
    CreateSsgiSRV(DXGI_FORMAT_R8G8B8A8_UNORM,            mGIHorizontalBlur, mSrvAlloc.Cpu(2));
    CreateSsgiSRV(DXGI_FORMAT_R8G8B8A8_UNORM,            mRandomVector,     mSrvAlloc.Cpu(3));
    CreateSsgiSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS,     depthStencilBuffer, mSrvAlloc.Cpu(4));
}
```

Update `CreateSsgiRTV` and `CreateSsgiSRV` signatures to take `D3D12_CPU_DESCRIPTOR_HANDLE` by value (same pattern as Task 5 Step 2 for GBuffer).

---

## Task 8: Update SSS.h/.cpp

**Files:**
- Modify: `Render/Resources/SSS.h`
- Modify: `Render/Resources/SSS.cpp`

SSS has 2 RTVs and 4 SRVs:
- RTV[0]=SSS, [1]=SSSBlur
- SRV[0]=SSS, [1]=SSSBlur, [2]=RandomVector, [3]=Depth

- [ ] **Step 1: Replace SSS.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class SSS
{
public:
    SSS(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    SSS(const SSS& rhs) = delete;
    SSS& operator=(const SSS& rhs) = delete;
    ~SSS() = default;

    UINT SSSWidth()  const;
    UINT SSSHeight() const;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSS()          const { return mSSS; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetSSSBlur()      const { return mSSSBlur; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetRandomVector() const { return mRandomVector; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSCpuRtv()    const { return mRtvAlloc.Cpu(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSSBlurCpuRtv() const { return mRtvAlloc.Cpu(1); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSGpuSrv()         const { return mSrvAlloc.Gpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSSSBlurGpuSrv()     const { return mSrvAlloc.Gpu(1); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetRandomVectorGpuSrv() const { return mSrvAlloc.Gpu(2); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetDepthGpuSrv()       const { return mSrvAlloc.Gpu(3); }

    void BuildDescriptors(ID3D12Device* device,
                          Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                          class DescriptorHeap& rtvHeap,
                          class DescriptorHeap& srvHeap);
    void RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer);
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);

    void CreateSSSTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateSSSRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateSSSSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);
    void BuildResources();

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mSSS;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSSSBlur;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVector;
    Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorUploadBuffer;

    DescriptorAllocation mRtvAlloc;  // 2 RTVs: SSS(0), SSSBlur(1)
    DescriptorAllocation mSrvAlloc;  // 4 SRVs: SSS(0), SSSBlur(1), RandomVec(2), Depth(3)

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};
```

- [ ] **Step 2: Update SSS.cpp — BuildDescriptors, RebuildDescriptors, helper signatures**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void SSS::BuildDescriptors(ID3D12Device* device,
                            Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer,
                            DescriptorHeap& rtvHeap,
                            DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(2);
    mSrvAlloc = srvHeap.Allocate(4);
    RebuildDescriptors(depthStencilBuffer);
}
```

Replace `RebuildDescriptors`:
```cpp
void SSS::RebuildDescriptors(Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CreateSSSRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSS,    mRtvAlloc.Cpu(0));
    CreateSSSRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mSSSBlur, mRtvAlloc.Cpu(1));

    CreateSSSSRV(DXGI_FORMAT_R8G8B8A8_UNORM,           mSSS,          mSrvAlloc.Cpu(0));
    CreateSSSSRV(DXGI_FORMAT_R8G8B8A8_UNORM,           mSSSBlur,      mSrvAlloc.Cpu(1));
    CreateSSSSRV(DXGI_FORMAT_R8G8B8A8_UNORM,           mRandomVector, mSrvAlloc.Cpu(2));
    CreateSSSSRV(DXGI_FORMAT_R24_UNORM_X8_TYPELESS,    depthStencilBuffer, mSrvAlloc.Cpu(3));
}
```

Update `CreateSSSRTV` and `CreateSSSSRV` to take `D3D12_CPU_DESCRIPTOR_HANDLE` by value.

---

## Task 9: Update Lighting.h/.cpp

**Files:**
- Modify: `Render/Resources/Lighting.h`
- Modify: `Render/Resources/Lighting.cpp`

Lighting has 1 RTV and 1 SRV:
- RTV[0]=Lighting, SRV[0]=Lighting

- [ ] **Step 1: Replace Lighting.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class Lighting
{
public:
    Lighting(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Lighting(const Lighting& rhs) = delete;
    Lighting& operator=(const Lighting& rhs) = delete;
    ~Lighting() = default;

    UINT LightingWidth()  const;
    UINT LightingHeight() const;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetLighting() const { return mLighting; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetLightingCpuRtv() const { return mRtvAlloc.Cpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetLightingGpuSrv() const { return mSrvAlloc.Gpu(0); }

    void BuildDescriptors(ID3D12Device* device, class DescriptorHeap& rtvHeap, class DescriptorHeap& srvHeap);
    void RebuildDescriptors();
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildResources();

    void CreateLightingTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateLightingRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateLightingSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mLighting;

    DescriptorAllocation mRtvAlloc;  // 1 RTV: Lighting(0)
    DescriptorAllocation mSrvAlloc;  // 1 SRV: Lighting(0)

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};
```

- [ ] **Step 2: Update Lighting.cpp**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void Lighting::BuildDescriptors(ID3D12Device* device, DescriptorHeap& rtvHeap, DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(1);
    mSrvAlloc = srvHeap.Allocate(1);
    RebuildDescriptors();
}
```

Replace `RebuildDescriptors`:
```cpp
void Lighting::RebuildDescriptors()
{
    CreateLightingRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mLighting, mRtvAlloc.Cpu(0));
    CreateLightingSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mLighting, mSrvAlloc.Cpu(0));
}
```

Update `CreateLightingRTV` and `CreateLightingSRV` to take `D3D12_CPU_DESCRIPTOR_HANDLE` by value.

---

## Task 10: Update Composite.h/.cpp

**Files:**
- Modify: `Render/Resources/Composite.h`
- Modify: `Render/Resources/Composite.cpp`

Composite has 1 RTV and 1 SRV:
- RTV[0]=Composite, SRV[0]=Composite

- [ ] **Step 1: Replace Composite.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class Composite
{
public:
    Composite(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
    Composite(const Composite& rhs) = delete;
    Composite& operator=(const Composite& rhs) = delete;
    ~Composite() = default;

    UINT CompositeWidth()  const;
    UINT CompositeHeight() const;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetComposite() const { return mComposite; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCompositeCpuRtv() const { return mRtvAlloc.Cpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetCompositeGpuSrv() const { return mSrvAlloc.Gpu(0); }

    void BuildDescriptors(ID3D12Device* device, class DescriptorHeap& rtvHeap, class DescriptorHeap& srvHeap);
    void RebuildDescriptors();
    void OnResize(UINT newWidth, UINT newHeight);
    void BuildResources();

    void CreateCompositeTexture(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
    void CreateCompositeRTV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void CreateCompositeSRV(const DXGI_FORMAT& format, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

private:
    ID3D12Device* md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Resource> mComposite;

    DescriptorAllocation mRtvAlloc;  // 1 RTV: Composite(0)
    DescriptorAllocation mSrvAlloc;  // 1 SRV: Composite(0)

    UINT mRenderTargetWidth  = 0;
    UINT mRenderTargetHeight = 0;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT     mScissorRect;
};
```

- [ ] **Step 2: Update Composite.cpp**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void Composite::BuildDescriptors(ID3D12Device* device, DescriptorHeap& rtvHeap, DescriptorHeap& srvHeap)
{
    mRtvAlloc = rtvHeap.Allocate(1);
    mSrvAlloc = srvHeap.Allocate(1);
    RebuildDescriptors();
}
```

Replace `RebuildDescriptors`:
```cpp
void Composite::RebuildDescriptors()
{
    CreateCompositeRTV(DXGI_FORMAT_R8G8B8A8_UNORM, mComposite, mRtvAlloc.Cpu(0));
    CreateCompositeSRV(DXGI_FORMAT_R8G8B8A8_UNORM, mComposite, mSrvAlloc.Cpu(0));
}
```

Update `CreateCompositeRTV` and `CreateCompositeSRV` to take `D3D12_CPU_DESCRIPTOR_HANDLE` by value.

---

## Task 11: Update ShadowResources.h/.cpp

**Files:**
- Modify: `Render/Resources/ShadowResources.h`
- Modify: `Render/Resources/ShadowResources.cpp`

ShadowResources allocates `numLights` SRVs and `numLights` DSVs, then delegates to `ShadowMap::BuildDescriptors(cpuSrv, gpuSrv, cpuDsv)` for each map. `ShadowMap` is unchanged — it receives computed handles from the allocation.

The public `GetStart*` accessors are preserved (used by none of the current render passes — only `shadowMaps[i]->Srv()` and `Dsv()` are used directly). The handle vectors are preserved for API compatibility.

- [ ] **Step 1: Replace ShadowResources.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"
#include "../Resources/ShadowMap.h"

class ShadowResources
{
public:
    ShadowResources(ID3D12Device* device);
    ShadowResources(const ShadowResources& rhs) = delete;
    ShadowResources& operator=(const ShadowResources& rhs) = delete;
    ~ShadowResources() = default;

    void BuildDescriptors(ID3D12Device* device,
                          int numLights,
                          class DescriptorHeap& srvHeap,
                          class DescriptorHeap& dsvHeap);

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuSrv() const { return mSrvAlloc.Cpu(0); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return mSrvAlloc.Gpu(0); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpuDsv() const { return mDsvAlloc.Cpu(0); }

    std::vector<std::unique_ptr<ShadowMap>> shadowMaps;

private:
    ID3D12Device* md3dDevice;

    DescriptorAllocation mSrvAlloc;  // numLights SRVs
    DescriptorAllocation mDsvAlloc;  // numLights DSVs
};
```

- [ ] **Step 2: Update ShadowResources.cpp**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void ShadowResources::BuildDescriptors(ID3D12Device* device,
                                        int numLights,
                                        DescriptorHeap& srvHeap,
                                        DescriptorHeap& dsvHeap)
{
    mSrvAlloc = srvHeap.Allocate(numLights);
    mDsvAlloc = dsvHeap.Allocate(numLights);

    for (int i = 0; i < numLights; i++)
    {
        shadowMaps.push_back(std::make_unique<ShadowMap>(md3dDevice, 2048, 2048));
        shadowMaps[i]->BuildDescriptors(
            CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvAlloc.Cpu(i)),
            CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvAlloc.Gpu(i)),
            CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvAlloc.Cpu(i)));
    }
}
```

---

## Task 12: Update RadianceResources.h/.cpp

**Files:**
- Modify: `Render/Resources/RadianceResources.h`
- Modify: `Render/Resources/RadianceResources.cpp`

RadianceResources allocates `2 * numLights` RTVs and `2 * numLights` SRVs. Layout: diffuse at [0..N-1], specular at [N..2N-1] in each allocation. The individual `Radiance` objects still call `BuildDiffuseDescriptors` / `BuildSpecularDescriptors` (which take cursor handles by ref) — we construct temporary CD3DX12 handles from the allocation and pass them, discarding the advancement.

`GetStartGpuSrv()` returns `mSrvAlloc.Gpu(0)` (the first diffuse SRV, as callers expect).

- [ ] **Step 1: Replace RadianceResources.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"
#include "../Resources/Radiance.h"

class RadianceResources
{
public:
    RadianceResources(ID3D12Device* device);
    RadianceResources(const RadianceResources& rhs) = delete;
    RadianceResources& operator=(const RadianceResources& rhs) = delete;
    ~RadianceResources() = default;

    void BuildDescriptors(ID3D12Device* device,
                          int numLights,
                          UINT width,
                          UINT height,
                          class DescriptorHeap& srvHeap,
                          class DescriptorHeap& rtvHeap);

    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return mSrvAlloc.Gpu(0); }

    std::vector<std::unique_ptr<Radiance>> radianceMaps;

private:
    ID3D12Device* md3dDevice;

    DescriptorAllocation mSrvAlloc;  // 2*numLights SRVs: diffuse[0..N-1], specular[N..2N-1]
    DescriptorAllocation mRtvAlloc;  // 2*numLights RTVs: diffuse[0..N-1], specular[N..2N-1]
};
```

- [ ] **Step 2: Update RadianceResources.cpp**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void RadianceResources::BuildDescriptors(ID3D12Device* device,
                                          int numLights,
                                          UINT width,
                                          UINT height,
                                          DescriptorHeap& srvHeap,
                                          DescriptorHeap& rtvHeap)
{
    mSrvAlloc = srvHeap.Allocate(2 * numLights);
    mRtvAlloc = rtvHeap.Allocate(2 * numLights);

    for (int i = 0; i < numLights; i++)
    {
        radianceMaps.push_back(std::make_unique<Radiance>(md3dDevice, width, height));
    }

    // Diffuse: slots 0..numLights-1 in each allocation
    for (int i = 0; i < numLights; i++)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtv(mRtvAlloc.Cpu(i));
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrv(mSrvAlloc.Cpu(i));
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrv(mSrvAlloc.Gpu(i));
        radianceMaps[i]->BuildDiffuseDescriptors(cpuRtv, cpuSrv, gpuSrv, mRtvAlloc.stride, mSrvAlloc.stride);
    }

    // Specular: slots numLights..2*numLights-1 in each allocation
    for (int i = 0; i < numLights; i++)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtv(mRtvAlloc.Cpu(numLights + i));
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrv(mSrvAlloc.Cpu(numLights + i));
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrv(mSrvAlloc.Gpu(numLights + i));
        radianceMaps[i]->BuildSpecularDescriptors(cpuRtv, cpuSrv, gpuSrv, mRtvAlloc.stride, mSrvAlloc.stride);
    }
}
```

---

## Task 13: Update RenderTextures.h/.cpp

**Files:**
- Modify: `Render/Resources/RenderTextures.h`
- Modify: `Render/Resources/RenderTextures.cpp`

RenderTextures allocates N SRVs where N = `textureData.size()`. `GetStartGpuSrv()` returns the first slot. The per-element vector accessors are removed (not used by any render pass).

- [ ] **Step 1: Replace RenderTextures.h**

```cpp
#pragma once
#include "../../D3D12/D3DUtil.h"
#include "../../D3D12/DescriptorAllocation.h"
#include "../Resources/FrameResource.h"

class RenderTextures
{
public:
    RenderTextures(ID3D12Device* device);
    RenderTextures(const RenderTextures& rhs) = delete;
    RenderTextures& operator=(const RenderTextures& rhs) = delete;
    ~RenderTextures() = default;

    void BuildDescriptors(ID3D12Device* device,
                          class DescriptorHeap& srvHeap,
                          const std::map<std::string, std::string>& textureData,
                          const std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>>& textures);

    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpuSrv() const { return mSrvAlloc.Gpu(0); }
    size_t GetCount() const { return mSrvAlloc.count; }

private:
    ID3D12Device* md3dDevice;

    DescriptorAllocation mSrvAlloc;  // N contiguous SRVs (N = textureData.size())
};
```

- [ ] **Step 2: Update RenderTextures.cpp**

Add include:
```cpp
#include "../../D3D12/DescriptorHeap.h"
```

Replace `BuildDescriptors`:
```cpp
void RenderTextures::BuildDescriptors(ID3D12Device* device,
                                       DescriptorHeap& srvHeap,
                                       const std::map<std::string, std::string>& textureData,
                                       const std::unordered_map<std::string, std::shared_ptr<std::pair<Texture, UINT>>>& textures)
{
    mSrvAlloc = srvHeap.Allocate((UINT)textureData.size());

    UINT i = 0;
    for (const auto& entry : textureData)
    {
        auto texResource = textures.at(entry.first)->first.Resource;
        assert(texResource != nullptr);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Format = texResource->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = texResource->GetDesc().MipLevels;
        device->CreateShaderResourceView(texResource.Get(), &srvDesc, mSrvAlloc.Cpu(i));
        i++;
    }
}
```

---

## Task 14: Update SetRenderPassResources.cpp — then build

**Files:**
- Modify: `Build/SetRenderPassResources.cpp`

This is the final piece. The cursor threading is removed entirely. Each call is order-independent.

- [ ] **Step 1: Replace SetRenderPassResources.cpp**

```cpp
#include "../EngineApp.h"

void EngineApp::SetRenderPassResources()
{
    mShadowPassCBs.resize(dynamicLights.GetNumLights());
    mRadianceCBs.resize(dynamicLights.GetNumLights());

    mGBuffer = std::make_unique<GBuffer>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mGBuffer->BuildDescriptors(md3dDevice.Get(), mPassRtvHeap, mPassSrvHeap);

    mSsao = std::make_unique<Ssao>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsao->BuildDescriptors(md3dDevice.Get(), GetDepthBuffer(), mPassRtvHeap, mPassSrvHeap);

    mLighting = std::make_unique<Lighting>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mLighting->BuildDescriptors(md3dDevice.Get(), mPassRtvHeap, mPassSrvHeap);

    mSsgi = std::make_unique<Ssgi>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSsgi->BuildDescriptors(md3dDevice.Get(), GetDepthBuffer(), mPassRtvHeap, mPassSrvHeap);

    mSss = std::make_unique<SSS>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mSss->BuildDescriptors(md3dDevice.Get(), GetDepthBuffer(), mPassRtvHeap, mPassSrvHeap);

    mComposite = std::make_unique<Composite>(md3dDevice.Get(), mCommandList.Get(), mClientWidth, mClientHeight);
    mComposite->BuildDescriptors(md3dDevice.Get(), mPassRtvHeap, mPassSrvHeap);

    mRadianceResources = std::make_unique<RadianceResources>(md3dDevice.Get());
    mRadianceResources->BuildDescriptors(md3dDevice.Get(), dynamicLights.GetNumLights(),
        mClientWidth, mClientHeight, mPassSrvHeap, mPassRtvHeap);

    mRenderTextures = std::make_unique<RenderTextures>(md3dDevice.Get());
    mRenderTextures->BuildDescriptors(md3dDevice.Get(), mPassSrvHeap,
        mAssets->mTextureData, mAssets->mTextures);

    mShadowResources = std::make_unique<ShadowResources>(md3dDevice.Get());
    mShadowResources->BuildDescriptors(md3dDevice.Get(), dynamicLights.GetNumLights(),
        mPassSrvHeap, mPassDsvHeap);
}
```

- [ ] **Step 2: Build the project**

Run the full build command. Expected: **0 errors**. Fix any errors before proceeding — common issues:
- Missing `#include "../../D3D12/DescriptorHeap.h"` in a resource .cpp
- Leftover `CD3DX12_CPU_DESCRIPTOR_HANDLE&` parameter somewhere not yet updated
- Lingering reference to `renderPassRtvHeap` / `renderPassSrvHeap` / `mSrvDescriptorHeap`

- [ ] **Step 3: Commit**

```bash
git add -u
git commit -m "feat: replace descriptor cursor threading with DescriptorHeap bump allocator"
```

---

## Task 15: Runtime verification + final commit

**Files:** none

- [ ] **Step 1: Launch the engine**

Open `OpenResearchEngine.sln` in Visual Studio 2022 and run via F5 (Debug). The engine should launch and render the scene. Verify:
- G-Buffer pass renders all 7 MRT targets (no black or corrupted G-Buffer channels)
- SSAO occlusion appears correctly
- Shadow maps render correctly for all lights
- Lighting pass combines G-Buffer, SSAO, shadows, and radiance correctly
- ImGui overlay is visible
- Window resize (drag the window border) works without crash — RebuildDescriptors paths still function

- [ ] **Step 2: Final commit**

```bash
git add -u
git commit -m "feat: descriptor heap refactor — verified runtime correctness"
```

---

## Self-Review Checklist

**Spec coverage:**
- [x] `DescriptorAllocation` value type with `Cpu(i)` / `Gpu(i)` / `IsValid()` — Task 2
- [x] `DescriptorHeap` bump allocator with `Create()` / `Allocate()` — Task 3
- [x] Three new EngineApp heap members (`mPassRtvHeap`, `mPassSrvHeap`, `mPassDsvHeap`) — Task 4
- [x] `mDsvHeap` reduced to capacity 1 — Task 4
- [x] `renderPassRtvHeap` / `renderPassSrvHeap` removed from D3DApp — Task 4
- [x] `SetRenderPassResources` cursor threading removed — Task 14
- [x] All 9 resource classes updated — Tasks 5–13
- [x] `Render.cpp` updated to use `mPassSrvHeap.Heap()` — Task 4
- [x] New `.cpp` added to vcxproj — Task 3
- [x] Resize path preserved: `RebuildDescriptors` signatures unchanged, allocations are permanent — Tasks 5–10
- [x] ImGui heap unchanged — Task 4

**Type consistency check:**
- `DescriptorAllocation::Cpu(UINT i)` returns `D3D12_CPU_DESCRIPTOR_HANDLE` — used directly in `CreateGBufferRTV(D3D12_CPU_DESCRIPTOR_HANDLE)` etc. ✓
- `DescriptorHeap` forward declarations match actual class name everywhere ✓
- `DescriptorHeap& rtvHeap` / `DescriptorHeap& srvHeap` / `DescriptorHeap& dsvHeap` parameter names are consistent across Tasks 4–14 ✓
- `mPassRtvHeap`, `mPassSrvHeap`, `mPassDsvHeap` — same names in EngineApp.h (Task 4) and SetRenderPassResources.cpp (Task 14) ✓
