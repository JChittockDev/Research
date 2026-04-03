# Descriptor Heap Refactor — Design Spec

**Date:** 2026-04-02  
**Branch:** `feature/descriptor-heap-refactor`  
**Status:** Approved

---

## Problem

The current descriptor allocation pattern in `SetRenderPassResources.cpp` manually threads cursor handles (`cpuRtvHandle`, `cpuSrvHandle`, `gpuSrvHandle`, `cpuDsvHandle`) through every `BuildDescriptors()` call in a fixed order. Each call silently advances all cursors by an implicit count. This is brittle:

- Adding, removing, or reordering a resource call corrupts all subsequent descriptor addresses silently.
- `NumDescriptors = 64` is a magic number with no overflow detection.
- Resource classes (especially GBuffer) store large numbers of redundant individual handle members (GBuffer alone has 28), all of which are just `base + stride * i`.
- `RebuildDescriptors()` on resize must trust that its handles stored at startup are still valid.

---

## Solution Overview

Introduce two new types — `DescriptorAllocation` and `DescriptorHeap` — that replace manual cursor-threading with explicit, self-contained allocation calls. Each resource independently requests the descriptors it needs and stores the resulting allocation. Order of calls in `SetRenderPassResources` no longer matters.

---

## New Types

### `DescriptorAllocation` (`D3D12/DescriptorAllocation.h`, header-only)

A value type representing a contiguous range of descriptors within a heap.

```cpp
struct DescriptorAllocation {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = {};  // only valid for shader-visible heaps
    UINT count  = 0;
    UINT stride = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE Cpu(UINT i) const;  // cpuStart + i * stride
    D3D12_GPU_DESCRIPTOR_HANDLE Gpu(UINT i) const;  // gpuStart + i * stride; debug-asserts shader-visible
    bool IsValid() const { return count > 0; }
};
```

Accessors compute sub-handles by index. Passing `DescriptorAllocation` by value is safe — handles remain valid for the lifetime of the heap.

### `DescriptorHeap` (`D3D12/DescriptorHeap.h` / `D3D12/DescriptorHeap.cpp`)

A bump allocator over a single `ID3D12DescriptorHeap`.

```cpp
class DescriptorHeap {
public:
    void Create(ID3D12Device*, D3D12_DESCRIPTOR_HEAP_TYPE, UINT capacity, bool shaderVisible);
    DescriptorAllocation Allocate(UINT count);  // debug-asserts on overflow

    ID3D12DescriptorHeap* Heap()     const;
    UINT                  Stride()   const;
    UINT                  Used()     const;
    UINT                  Capacity() const;
private:
    ComPtr<ID3D12DescriptorHeap> mHeap;
    UINT mStride   = 0;
    UINT mNextFree = 0;
    UINT mCapacity = 0;
};
```

No de-allocation. All descriptors are allocated once at startup and remain valid for the application lifetime. Overflow is caught immediately in debug builds via `assert`.

No singleton — instances are owned directly by `EngineApp`.

---

## EngineApp Changes

### Members

D3DApp owns `mRtvHeap` and `mDsvHeap` as `ComPtr<ID3D12DescriptorHeap>` and uses them directly in `DepthStencilView()`, `OnResize()`, etc. — **we do not touch D3DApp**.

EngineApp currently has three heaps of its own: `renderPassRtvHeap`, `renderPassSrvHeap`, and `imGuiSrvDescriptorHeap`. Replace the first two with `DescriptorHeap` members and add a third for shadow DSVs (currently piggybacking on D3DApp's `mDsvHeap`):

```cpp
DescriptorHeap mPassRtvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_RTV,         capacity 32,  not shader-visible
DescriptorHeap mPassSrvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity 128, shader-visible
DescriptorHeap mPassDsvHeap;  // D3D12_DESCRIPTOR_HEAP_TYPE_DSV,         capacity 16,  not shader-visible
```

The ImGui heap remains a separate `ComPtr<ID3D12DescriptorHeap>` — ImGui manages it externally.

D3DApp's `mDsvHeap` is reduced to capacity 1 (just the main depth buffer); shadow map DSVs move into `mPassDsvHeap`. D3DApp's `DepthStencilView()` still works — it always returns index 0.

### `PopulateDescriptorHeaps.cpp`

D3DApp's `mRtvHeap` and `mDsvHeap` are created as before (the override still calls `CreateDescriptorHeap` for them, with `mDsvHeap` reduced to 1 descriptor). Additionally calls `Create()` on the three new `DescriptorHeap` members.

### `SetRenderPassResources.cpp`

Before (order-dependent cursor threading):
```cpp
CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle(renderPassRtvHeap->GetCPUDescriptorHandleForHeapStart());
mGBuffer->BuildDescriptors(cpuRtvHandle, cpuSrvHandle, gpuSrvHandle, mRtvDescriptorSize, mCbvSrvUavDescriptorSize);
mSsao->BuildDescriptors(GetDepthBuffer(), cpuRtvHandle, cpuSrvHandle, ...);
// ...
```

After (order-independent):
```cpp
mGBuffer->BuildDescriptors(md3dDevice.Get(), mPassRtvHeap, mPassSrvHeap);
mSsao->BuildDescriptors(md3dDevice.Get(), mPassRtvHeap, mPassSrvHeap);
mShadowResources->BuildDescriptors(md3dDevice.Get(), dynamicLights.GetNumLights(), mPassSrvHeap, mPassDsvHeap);
// ...
```

Each call is self-contained. Render pass resource objects are set up in `SetRenderPassResources` and `mPassSrvHeap.Heap()` is passed to the command list's `SetDescriptorHeaps` in `Render.cpp`.

---

## Resource Class Changes

All resource classes follow the same pattern: replace individual handle members with `DescriptorAllocation` members, update `BuildDescriptors()` to take heap refs and call `Allocate()`, and simplify `RebuildDescriptors()`.

### GBuffer

```cpp
// Before: 28 individual handle members
// After:
DescriptorAllocation mRtvAlloc;  // 7 contiguous RTVs (Position, Normal, ViewNormal, AlbedoSpec, Reflection, MaterialId, Tangent)
DescriptorAllocation mSrvAlloc;  // 7 contiguous SRVs

// Accessors become one-liners (public API unchanged):
D3D12_CPU_DESCRIPTOR_HANDLE GetPositionCpuRtv() const { return mRtvAlloc.Cpu(0); }
D3D12_CPU_DESCRIPTOR_HANDLE GetNormalCpuRtv()   const { return mRtvAlloc.Cpu(1); }
// etc.

// BuildDescriptors signature:
void BuildDescriptors(ID3D12Device*, DescriptorHeap& rtvHeap, DescriptorHeap& srvHeap);

// RebuildDescriptors signature (no heap args — slots are permanent):
void RebuildDescriptors(ID3D12Device*);
```

### ShadowResources

```cpp
DescriptorAllocation mSrvAlloc;  // numLights SRVs
DescriptorAllocation mDsvAlloc;  // numLights DSVs

void BuildDescriptors(ID3D12Device*, int numLights, DescriptorHeap& srvHeap, DescriptorHeap& dsvHeap);
```

### RadianceResources

```cpp
DescriptorAllocation mSrvAlloc;  // numLights SRVs
DescriptorAllocation mRtvAlloc;  // numLights RTVs (per Radiance map)

void BuildDescriptors(ID3D12Device*, int numLights, UINT width, UINT height, DescriptorHeap& srvHeap, DescriptorHeap& rtvHeap);
```

### RenderTextures

```cpp
DescriptorAllocation mSrvAlloc;  // numTextures SRVs

void BuildDescriptors(ID3D12Device*, DescriptorHeap& srvHeap, ...);
```

### Ssao, Ssgi, SSS, Lighting, Composite

Each has a small fixed descriptor count. Each gets 1–2 `DescriptorAllocation` members replacing their individual handles. Signatures follow the same pattern.

---

## Resize Handling

On window resize, resource classes recreate their GPU textures and re-write descriptors into existing allocated slots:

```cpp
void GBuffer::RebuildDescriptors(ID3D12Device* device) {
    // re-create textures at new size (BuildResources())
    // re-write descriptors into permanent slots:
    for (int i = 0; i < 7; i++) {
        device->CreateRenderTargetView(textures[i], ..., mRtvAlloc.Cpu(i));
        device->CreateShaderResourceView(textures[i], ..., mSrvAlloc.Cpu(i));
    }
}
```

No re-allocation from the heap. The `DescriptorAllocation` stored at init is permanent.

---

## File Changes Summary

| File | Change |
|------|--------|
| `D3D12/DescriptorAllocation.h` | **New** — value type |
| `D3D12/DescriptorHeap.h` | **New** — allocator header |
| `D3D12/DescriptorHeap.cpp` | **New** — allocator implementation |
| `EngineApp.h` | Replace `renderPassRtvHeap`/`renderPassSrvHeap` with 3 `DescriptorHeap` members |
| `Build/PopulateDescriptorHeaps.cpp` | Add `Create()` calls for 3 new heaps; reduce D3DApp `mDsvHeap` to capacity 1 |
| `Build/SetRenderPassResources.cpp` | Remove cursor threading; pass heap refs to each `BuildDescriptors()` |
| `Render/Manager/Render.cpp` | Update `SetDescriptorHeaps` to use `mPassSrvHeap.Heap()` |
| `Render/Resources/GBuffer.h/.cpp` | Replace 28 handles with 2 `DescriptorAllocation`; update `BuildDescriptors`/`RebuildDescriptors` |
| `Render/Resources/ShadowResources.h/.cpp` | Replace handle members with 2 `DescriptorAllocation` |
| `Render/Resources/RadianceResources.h/.cpp` | Replace handle members with 2 `DescriptorAllocation` |
| `Render/Resources/RenderTextures.h/.cpp` | Replace handle vectors with 1 `DescriptorAllocation` |
| `Render/Resources/Ssao.h/.cpp` | Replace handle members with `DescriptorAllocation` |
| `Render/Resources/Ssgi.h/.cpp` | Replace handle members with `DescriptorAllocation` |
| `Render/Resources/SSS.h/.cpp` | Replace handle members with `DescriptorAllocation` |
| `Render/Resources/Lighting.h/.cpp` | Replace handle members with `DescriptorAllocation` |
| `Render/Resources/Composite.h/.cpp` | Replace handle members with `DescriptorAllocation` |
| `OpenResearchEngine.vcxproj` | Add `D3D12/DescriptorHeap.cpp` |

---

## Invariants

- All `Allocate()` calls happen during `SetRenderPassResources()` at startup, before any frame renders.
- `DescriptorAllocation` values are immutable after `BuildDescriptors()` returns.
- Only `mSrvHeap` is shader-visible; `mRtvHeap` and `mDsvHeap` are CPU-only.
- `DescriptorAllocation::Gpu()` debug-asserts that the parent heap was shader-visible.
- ImGui retains its own separate `ComPtr<ID3D12DescriptorHeap>`.
