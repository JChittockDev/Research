# Plan 4: Update* Extraction & Subsystem Integration

**Date:** 2026-03-30
**Status:** Approved

## Goal

Finish the EngineApp God Class breakup. Wire the three subsystems (`AssetManager`, `RenderPipeline`, `AnimationSystem`) into `EngineApp` and convert the remaining `Update*` methods into free functions. After this plan `EngineApp` owns only three subsystems, a `SceneState`, a `Camera`, and frame resource management.

---

## Context

Plans 1–3 produced three complete subsystem classes that are not yet integrated:

| Class | Location | Status |
|-------|----------|--------|
| `RenderPipeline` + 13 `IRenderPass` impls | `Render/` | Complete, not wired |
| `AnimationSystem` | `Animation/` | Complete, not wired |
| `AssetManager` | `Assets/` | `.h` correct; `.cpp` broken (uses deleted `Mesh`/`MeshGeometry`) |

`EngineApp` still owns all resource maps, root signatures, PSOs, render pass methods, and Update* methods. `Build/*.cpp` (16 files) are `EngineApp::` methods that duplicate what `AssetManager::Build()` should do and are the up-to-date version. `Update/*.cpp` (10 files) are `EngineApp::` methods.

---

## Target Architecture

```
EngineApp
├── std::unique_ptr<AssetManager>    mAssets
├── std::unique_ptr<RenderPipeline>  mRenderPipeline
├── std::unique_ptr<AnimationSystem> mAnimationSystem
├── SceneState                       mSceneState
├── Camera                           mCamera
└── FrameResource management + D3DApp infrastructure
```

**Frame loop:**
```
Update(gt):
  OnKeyboardInput / ImGui new frame / frame resource cycling (unchanged)
  UpdateLights / UpdateObjectCBs / mAnimationSystem->Update / UpdateMaterialBuffer
  UpdateShadowPassCB / UpdateMainPassCB / UpdateSssCB / UpdateScreenSpaceCB / UpdateRadiancePassCB

Draw(gt):
  RenderContext ctx = BuildRenderContext(...)
  mRenderPipeline->Execute(ctx, mCurrFrameResource)
  ImGui render + present (unchanged)
```

---

## New Type: SceneState

**File:** `Common/SceneState.h`

Holds per-frame values that Update* free functions need and that are not owned by `AssetManager` or `FrameResource`.

```cpp
struct SceneState {
    PassConstants                  mainPassCB;
    std::vector<PassConstants>     shadowPassCBs;
    std::vector<RadianceConstants> radianceCBs;
    DirectX::BoundingSphere        sceneBounds;
    DynamicLights                  lights;
    int                            clientWidth  = 0;
    int                            clientHeight = 0;
};
```

`EngineApp` holds one instance as `mSceneState` and passes it by reference to Update* free functions and into `RenderContext`.

---

## RenderContext Additions

Two fields added to `Render/RenderContext.h`:

```cpp
const AssetManager* assets     = nullptr;  // passes pull root sigs, PSOs, textures
const SceneState*   sceneState = nullptr;  // passes read pass CBs and lights
```

---

## Step 1: AssetManager Integration

### What moves to AssetManager

Add these public maps to `AssetManager.h` (currently only on `EngineApp`):

```cpp
std::vector<std::shared_ptr<RenderItem>>                                    mRenderItems;
std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mRenderItemLayers;
std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mMeshRenderItemMap;
std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mDeformedRenderItemMap;
std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mDirectionalLightRenderItemMap;
std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>   mSpotLightRenderItemMap;
UINT ObjectCBIndex   = 0;
UINT SkinnedCBIndex  = 0;
UINT BlendCBIndex    = 0;

// Serialized level data (currently on EngineApp)
std::unordered_map<std::string, std::unordered_map<std::string, ItemData>>        mLevelRenderItems;
std::unordered_map<std::string, std::unordered_map<std::string, PBRMaterialData>> mLevelMaterials;
std::unordered_map<std::string, std::unordered_map<std::string, LightData>>       mLevelLights;
```

`OnnxModelResource` also moves into `AssetManager` (it takes PSOs and root signatures already owned by AssetManager).

### AssetManager::Build()

Replaces `EngineApp::BuildScene()`. The body is ported from `Build/*.cpp` in this order:

```
SerializeLevel → SetRootSignatures → CompileShaders → ImportTextures
→ PushGenericMesh → PushMesh → PushMaterials → PushRenderItems → SetPipelineStates
→ OnnxModelResource::Initialize
```

`AssetManager::Build()` takes no parameters — it reads level data from `mBasePath` internally (same as `SerializeLevel` did via `GetFullPath`).

### AssetManager.cpp fixes

- Remove `#include "../Render/Resources/Mesh.h"`
- `PushMesh()` ported from `Build/PushMesh.cpp` (new `RenderMeshAsset::Load()` pattern)
- `PushGenericMesh()` ported from `Build/PushGenericMesh.cpp` (uses new static-batch / RenderMeshAsset path if applicable, otherwise the GeometryGenerator path updated to not use `MeshGeometry`)
- All other private methods (`CompileShaders`, `SetRootSignatures`, `SetPipelineStates`, `ImportTextures`, `PushMaterials`, `SerializeLevel`, `PushLights`, `PushRenderItems`, `SetFenceResources`, `PopulateDescriptorHeaps`, `MakePhysicsDeformerResources`) ported from their `Build/*.cpp` equivalents

### EngineApp changes

`BuildScene()` becomes:
```cpp
void EngineApp::BuildScene() {
    mAssets = std::make_unique<AssetManager>(md3dDevice, mCommandList, GetBasePath(), ...);
    mAssets->Build();
    SetRenderPassResources();  // stays on EngineApp — needs D3DApp resize handles
}
```

All resource maps, root signature members, PSO map, level data maps, counter indices, and `OnnxModelResource` are removed from `EngineApp.h`.

### Build/ directory deleted

All 16 files in `Build/` are deleted after their logic is ported to `AssetManager`.

---

## Step 2: RenderPipeline + AnimationSystem

### RenderPipeline

`EngineApp::Render()` becomes:
```cpp
void EngineApp::Render(FrameResource* fr) {
    RenderContext ctx = BuildRenderContext(
        mCommandList.Get(), mAssets.get(), &mSceneState,
        CurrentBackBuffer().Get(), CurrentBackBufferView(),
        GetDepthStencilView(), mScreenViewport, mScissorRect);
    mRenderPipeline->Execute(ctx, fr);
}
```

Pass instances are created in `EngineApp::Initialize()` after `mAssets` is built, since they need root signatures and PSOs from `mAssets`:
```cpp
mRenderPipeline = std::make_unique<RenderPipeline>();
mRenderPipeline->AddPass(std::make_unique<AnimationPass>(*mAssets));
mRenderPipeline->AddPass(std::make_unique<PhysicsPass>(*mAssets));
mRenderPipeline->AddPass(std::make_unique<ShadowPass>(*mAssets));
// ... all 13 passes
```

Each pass constructor takes `AssetManager&` and stores the root signatures and PSOs it needs by pointer/reference. The existing `EngineApp::ShadowPass()`, `EngineApp::GBufferPass()` etc. method bodies move into the corresponding `IRenderPass::Execute()` implementations. Those `EngineApp::` render methods are deleted.

### AnimationSystem

`UpdateAnimCBs` (`Update/UpdateAnimConstants.cpp`) is deleted. Its logic already lives in `AnimationSystem::Update()`. In `EngineApp::Update()`:
```cpp
mAnimationSystem->Update(gt, *mAssets, *mCurrFrameResource);
```
`mAnimationSystem` is constructed in `EngineApp::Initialize()` after `mAssets`:
```cpp
mAnimationSystem = std::make_unique<AnimationSystem>();
```

---

## Step 3: Update* Extraction

### New header

`Update/UpdateFunctions.h` — declares all free functions so `EngineApp.cpp` needs one include.

### Free function signatures

Each function takes only what it actually uses:

```cpp
// Update/UpdateRenderAssets.cpp  — top-level dispatcher (or removed; EngineApp::Update calls directly)
// Update/UpdateMainPass.cpp
void UpdateMainPassCB(const GameTimer&, const Camera&, SceneState&, FrameResource*);

// Update/UpdateShadowPass.cpp  (contains two functions)
void UpdateShadowTransform(const GameTimer&, SceneState&);
void UpdateShadowPassCB(const GameTimer&, SceneState&, FrameResource*);

// Update/UpdateLights.cpp
void UpdateLights(const GameTimer&, SceneState&, AssetManager&, FrameResource*);

// Update/UpdateRenderItems.cpp
void UpdateObjectCBs(const GameTimer&, AssetManager&, FrameResource*);

// Update/UpdateMaterialBuffers.cpp
void UpdateMaterialBuffer(const GameTimer&, AssetManager&, FrameResource*);

// Update/UpdateSSCB.cpp
void UpdateScreenSpaceCB(const GameTimer&, SceneState&, FrameResource*);

// Update/UpdateSSSCB.cpp
void UpdateSssCB(const GameTimer&, SceneState&, FrameResource*);

// Update/UpdateRadiancePass.cpp
void UpdateRadiancePassCB(const GameTimer&, SceneState&, AssetManager&, FrameResource*);
```

`UpdateRenderAssets.cpp` is deleted — the dispatcher is no longer needed since `EngineApp::Update()` calls each function directly (order is explicit and visible).

### EngineApp::Update() after Step 3

```cpp
void EngineApp::Update(const GameTimer& gt) {
    OnKeyboardInput(gt);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // frame resource cycling + fence wait (unchanged)

    ImGui::Begin("Scene");
    UpdateLights(gt, mSceneState, *mAssets, mCurrFrameResource);
    UpdateObjectCBs(gt, *mAssets, mCurrFrameResource);
    mAnimationSystem->Update(gt, *mAssets, *mCurrFrameResource);
    UpdateMaterialBuffer(gt, *mAssets, mCurrFrameResource);
    UpdateShadowTransform(gt, mSceneState);
    UpdateShadowPassCB(gt, mSceneState, mCurrFrameResource);
    UpdateMainPassCB(gt, mCamera, mSceneState, mCurrFrameResource);
    UpdateSssCB(gt, mSceneState, mCurrFrameResource);
    UpdateScreenSpaceCB(gt, mSceneState, mCurrFrameResource);
    UpdateRadiancePassCB(gt, mSceneState, *mAssets, mCurrFrameResource);
    ImGui::End();
}
```

### EngineApp.h after Plan 4

Declarations removed:
- All `UpdateXxx` methods
- All `BuildScene` sub-method declarations
- All render pass method declarations (`ShadowPass`, `GBufferPass`, etc.)
- All root signature members
- All resource map members
- `mPSOs`, `mShaders`, `mInputLayout`, `mSkinnedInputLayout`
- Level data maps
- Counter indices
- `mOnnxModelResource`

Members that remain:
```cpp
std::unique_ptr<AssetManager>    mAssets;
std::unique_ptr<RenderPipeline>  mRenderPipeline;
std::unique_ptr<AnimationSystem> mAnimationSystem;
SceneState                       mSceneState;
Camera                           mCamera;
POINT                            mLastMousePos;
// mSceneBounds → moved into SceneState.sceneBounds
int                              mCurrFrameResourceIndex;
FrameResource*                   mCurrFrameResource;
std::vector<std::shared_ptr<FrameResource>> mFrameResources;
ComPtr<ID3D12DescriptorHeap>     mSrvDescriptorHeap;
ComPtr<ID3D12DescriptorHeap>     imGuiSrvDescriptorHeap;
```

---

## Files Deleted

| File(s) | Reason |
|---------|--------|
| `Build/*.cpp` (16 files) | Logic ported to `AssetManager` |
| `Update/UpdateAnimConstants.cpp` | Replaced by `AnimationSystem::Update()` |
| `Update/UpdateRenderAssets.cpp` | Dispatcher no longer needed |

## Files Created

| File | Purpose |
|------|---------|
| `Common/SceneState.h` | Per-frame camera/CB/light state struct |
| `Update/UpdateFunctions.h` | Declares all Update* free functions |

## Files Modified

| File | Change |
|------|--------|
| `Assets/AssetManager.h` | Add render item maps, level data maps, OnnxModelResource |
| `Assets/AssetManager.cpp` | Full rewrite: port Build/ logic, fix Mesh.h dependency |
| `Render/RenderContext.h` | Add `assets` and `sceneState` pointers |
| `Render/Passes/*.cpp` (13 files) | Move `EngineApp::XxxPass()` bodies into `Execute()` |
| `Update/*.cpp` (8 files) | Remove `EngineApp::` prefix, update signatures |
| `EngineApp.h` | Slim to three subsystems + SceneState + Camera |
| `EngineApp.cpp` | Wire subsystems; slim `Update()`, `Draw()`, `BuildScene()` |
| `Animation/AnimationSystem.h/.cpp` | Verify interface, no changes expected |
