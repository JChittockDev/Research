# Mesh System Refactor — Design Spec

**Date:** 2026-03-29
**Branch:** refactor/render-pipeline
**Status:** Approved by user

---

## Problem Statement

The current mesh system has four core problems:

1. **All GPU simulation buffers are allocated for every deformable mesh**, even when `Simulation=false`. This wastes significant GPU memory (9+ buffers per mesh).
2. **`RenderItem` is a God Object** — it holds 19 offset/count fields, 3 overlapping animation controllers, a physics flag, and geometry data all in one struct. Per-instance deformation state is split across `SkinningController`, `BlendshapeController`, and `MeshAnimationResource` with no clear ownership.
3. **No instancing support** — every mesh is a unique draw call. There is no way to render N static copies of the same geometry efficiently.
4. **Simulation and render topology are mixed** in `MeshGeometry`, making it impossible to load one without the other.

---

## Design: Asset/Instance Split

Everything is divided into two layers:

- **Asset** — shared, loaded once per unique source file, GPU-resident after upload, immutable
- **Instance** — per render-item, owns mutable per-frame deformation state

---

## Asset Layer

### `RenderMeshAsset`

Replaces `MeshGeometry` for the render side. Created once per unique file path.

**Always present:**
- `VertexBufferGPU` + uploader
- `IndexBufferGPU` + uploader
- `VertexByteStride`, `IndexFormat`, `VertexBufferByteSize`, `IndexBufferByteSize`
- `DrawArgs` (`std::unordered_map<std::string, SubmeshGeometry>`)

**Present only if the source has bones (`hasSkinning`):**
- `SkinningBufferGPU` + uploader

**Present only if the source has morph targets (`hasBlendshapes`):**
- `BlendshapeBufferGPU` + uploader

No simulation buffers of any kind.

### `SimMeshAsset`

New class. Holds only physics topology. **Required by any `PhysicsDeformer`; never created otherwise.**

Created by re-importing the same source file with the sim-specific Assimp flags already in use (`aiProcess_RemoveComponent` stripping normals, tangents, colours, bone weights, animations; `aiProcess_JoinIdenticalVertices` to weld). This is unchanged from current behaviour — it is just isolated into its own class.

**Contents:**
- `SimMeshVertexBufferGPU` — welded sim-mesh vertex positions
- `SimMeshConstraintsBufferGPU` — edge rest-lengths (float per edge)
- `SimMeshConstraintIDsBufferGPU` — edge vertex-pair IDs (`Edge` struct)
- `SimMeshVertexNeighbourBufferGPU` — vertex adjacency for normal solve
- `SimMeshNullSolverAccumulationBufferGPU` — zero-initialised accumulation (reset each frame)
- `SimMeshNullSolverCountBufferGPU` — zero-initialised count (reset each frame)
- `SimMeshTransferBufferGPU` — sim-vert → render-vert index map
- `MeshTransferBufferGPU` — render-vert → sim-vert index map
- `SimMeshVertexColorBufferGPU` — per-sim-vertex mask (loaded from vertex colour texture)
- `TriangleAdjacencyBufferGPU` — triangle-neighbour indices for the **render** mesh (computed from render-mesh `segmentedIndices`); only needed by `PhysicsPass` post-simulation normal recalculation, so lives here rather than on `RenderMeshAsset`
- Sim subset data: per-submesh counts/starts for sim verts, indices, triangles, constraints

**`AssetManager` holds:**
```cpp
std::unordered_map<std::string, std::shared_ptr<RenderMeshAsset>> mRenderMeshAssets;
std::unordered_map<std::string, std::shared_ptr<SimMeshAsset>>    mSimMeshAssets;
```

`mGeometries`, `mMesh`, `mMeshAnimationResources`, `mSkinningControllers`, `mBlendshapeControllers` are removed.

---

## Instance Layer

### `IDeformer` Interface

```cpp
class IDeformer {
public:
    virtual ~IDeformer() = default;
    virtual void Execute(ID3D12GraphicsCommandList*, const DeformContext&) = 0;
    virtual DeformerType Type() const = 0;
};

enum class DeformerType { Skin, Blendshape, Physics };

// Passed to each IDeformer::Execute — avoids reaching into AssetManager/EngineApp
struct DeformContext {
    AssetManager*        Assets;        // for PSO / root signature lookup
    RenderMeshAsset*     MeshAsset;     // shared geometry
    UINT                 ObjCBIndex;    // object constant buffer index
    UINT                 SkinnedCBIndex;// skinning CB index (used by SkinDeformer)
    UINT                 BlendCBIndex;  // blend CB index (used by BlendshapeDeformer)
};
```

### Concrete Deformers

**`SkinDeformer`**
- Owns: `SkinnedVertexBufferGPU` (one per instance, size = renderMesh vertex count × `sizeof(Vertex)`)
- References: `SkinningController` (unchanged struct, holds bone matrices)
- `Execute()` dispatches the skinning compute shader, writing deformed verts into `SkinnedVertexBufferGPU`

**`BlendshapeDeformer`**
- Owns: `BlendedVertexBufferGPU` (one per instance)
- References: `BlendshapeController` (unchanged struct, holds blend weights)
- `Execute()` dispatches the blendshape compute shader, reading from either `SkinnedVertexBufferGPU` (if `SkinDeformer` ran first) or the asset's raw `VertexBufferGPU`

**`PhysicsDeformer`**
- Owns per-instance solver state buffers:
  - `SimMeshSkinnedVertexBufferGPU`
  - `SimMeshPreviousSkinnedVertexBufferGPU`
  - `SimMeshForceBufferGPU`
  - `SimMeshTensionBufferGPU`
  - `SimMeshSolverAccumulationBufferGPU`
  - `SimMeshSolverCountBufferGPU`
  - `SimMeshSolverVertexBufferGPU`
- References: `SimMeshAsset*` (non-owning, for topology buffers — mandatory, enforced at construction)
- `Execute()` dispatches the full PBD pipeline: MeshTransfer → Tension → Force → PreSolve → ConstraintSolve → PostSolve → SimMeshTransfer → TriangleNormals → VertexNormals

### `DeformationGraph`

```cpp
class DeformationGraph {
public:
    void AddDeformer(std::unique_ptr<IDeformer> d);
    void Execute(ID3D12GraphicsCommandList*, const DeformContext&);
    ID3D12Resource* OutputBuffer() const; // SkinnedVertexBufferGPU from last deformer, or nullptr for static

    // Owns the final transformed vertex buffer for normal passes
    ComPtr<ID3D12Resource> TransformedVertexBufferGPU;
    // ... uploader, size

private:
    std::vector<std::unique_ptr<IDeformer>> mDeformers;
};
```

The graph is always executed in declaration order: `Skin → Blendshape → Physics`. The graph owns `TransformedVertexBufferGPU` — the output that the GBuffer pass reads.

For **static meshes**, `DeformationGraph` is empty. `OutputBuffer()` returns `nullptr` and the GBuffer pass falls back to the asset's `VertexBufferGPU`.

### `MeshInstance`

```cpp
class MeshInstance {
public:
    MeshInstance(RenderMeshAsset* asset, DeformationGraph graph);
    RenderMeshAsset* Asset() const;
    DeformationGraph& Graph();
    ID3D12Resource* FinalVertexBuffer() const; // graph output, or asset VertexBufferGPU

private:
    RenderMeshAsset* mAsset;   // non-owning
    DeformationGraph mGraph;
};
```

Instancing: multiple `MeshInstance` objects may point to the same `RenderMeshAsset`. Each owns independent deformation state.

---

## Thin `RenderItem`

```cpp
struct RenderItem {
    DirectX::XMFLOAT4X4 World       = Math::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = Math::Identity4x4();
    int NumFramesDirty               = gNumFrameResources;
    UINT ObjCBIndex                  = -1;

    MeshInstance* Instance = nullptr;  // non-owning; null for GPU-batched statics
    Material*     Mat      = nullptr;
    std::string   SubsetName;          // key into Instance->Asset()->DrawArgs

    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
```

Offsets and counts are **not stored** on `RenderItem`. They are read at draw time from `Instance->Asset()->DrawArgs[SubsetName]`. The three CB index fields (`SkinnedCBIndex`, `BlendCBIndex`) are removed — they are owned by the individual deformers.

---

## GPU Instancing — `StaticBatch`

```cpp
class StaticBatch {
public:
    StaticBatch(RenderMeshAsset* asset, const std::string& subsetName, Material* mat);
    void AddInstance(const DirectX::XMFLOAT4X4& world, UINT materialIndex);
    void UploadInstanceBuffer(ID3D12Device*, ID3D12GraphicsCommandList*);
    void Draw(ID3D12GraphicsCommandList*) const;

private:
    RenderMeshAsset*  mAsset;
    std::string       mSubsetName;
    Material*         mMat;
    std::vector<DirectX::XMFLOAT4X4> mWorldMatrices;
    ComPtr<ID3D12Resource> mInstanceBufferGPU;
    ComPtr<ID3D12Resource> mInstanceBufferUploader;
};
```

`AssetManager::Build()` groups static (non-animated, non-simulated) render items with identical asset + subset + material into `StaticBatch` objects. Each batch renders with one `DrawIndexedInstanced` call. Items that cannot be batched (any deformer present) go through the single-draw `RenderItem` path.

`AssetManager` holds:
```cpp
std::vector<std::shared_ptr<StaticBatch>> mStaticBatches;
```

---

## Level JSON Changes

Current schema uses `"Deformable": true` as a single flag. The new schema uses two explicit flags and moves simulation mask to the item level.

### Old schema (current)
```json
"WalkingMan": {
  "Deformable": true,
  "Geometry": "Models/test.fbx",
  "Position": [...], "Rotation": [...], "Scale": [...],
  "RenderLayer": "Opaque",
  "Settings": {
    "Head": { "Material": "Skin", "Simulation": true, "SimulationMask": "Textures/mask.png" }
  }
}
```

### New schema
```json
"WalkingMan": {
  "Animated": true,
  "Simulated": true,
  "SimulationMask": "Textures/mask.png",
  "Geometry": "Models/test.fbx",
  "Position": [...], "Rotation": [...], "Scale": [...],
  "RenderLayer": "Opaque",
  "Settings": {
    "Head": { "Material": "Skin" },
    "LeftEyeball": {},
    "RightEyeball": {}
  }
}
```

**Rules:**
- `"Animated": true` — `AssetManager` loads a `RenderMeshAsset` with skinning/blendshape buffers, creates `SkinDeformer` and (if blendshapes exist) `BlendshapeDeformer`
- `"Simulated": true` — `AssetManager` also loads a `SimMeshAsset` for the same file and adds a `PhysicsDeformer` to the graph. Requires `"Animated": true`.
- `"SimulationMask"` — path to the vertex colour PNG used for sim constraints. Moves from per-subset `Settings` to item level, since it applies to the whole mesh.
- Static items (neither flag set) produce `RenderItem`s eligible for `StaticBatch` grouping.
- `Settings` per-subset: only `"Material"` remains. `"Simulation"` and `"SimulationMask"` fields are removed from subset settings.

**`DemoLevel.json` changes required:**
- `"WalkingMan"`: replace `"Deformable": true` with `"Animated": true`. Add `"Simulated": true` only if physics is needed for that item. Move `SimulationMask` from any subset settings to the item level.
- All static items (skyDome, floor, mirrorDome1, shaderDome1): add to no-change; they will automatically become `StaticBatch` candidates.

---

## `ItemData` / `RenderItemSettings` Changes (Structures.h)

```cpp
// Old
struct RenderItemSettings {
    bool Simulation = false;
    std::string SimulationMask = "none";
    std::string Material = "default";
};

struct ItemData {
    bool deformable;
    ...
    std::unordered_map<std::string, RenderItemSettings> settings;
};

// New
struct RenderItemSettings {
    std::string Material = "default";  // Simulation and SimulationMask removed
};

struct ItemData {
    bool animated  = false;
    bool simulated = false;
    std::string simulationMask = "none";  // moved from per-subset to item level
    ...
    std::unordered_map<std::string, RenderItemSettings> settings;
};
```

---

## Loading Flow

```
AssetManager::Build()
  → PushGenericMesh()          (unchanged — shapeGeo)
  → PushMesh(levelItems)
      for each item:
        if not already loaded: RenderMeshAsset::Load(filepath, ...)  → mRenderMeshAssets
        if item.simulated:     SimMeshAsset::Load(filepath, ...)     → mSimMeshAssets
  → PushMaterials()            (unchanged)
  → PushRenderItems(levelItems)
      for each item:
        build MeshInstance with empty DeformationGraph
        if item.animated:  add SkinDeformer, optionally BlendshapeDeformer
        if item.simulated: add PhysicsDeformer (takes mSimMeshAssets[filepath])
        create RenderItem per subset
      group static RenderItems into StaticBatches
```

`Mesh` class (the loader) is split:
- `RenderMeshAsset::Load(...)` — static factory, contains render-path Assimp import
- `SimMeshAsset::Load(...)` — static factory, contains sim-path Assimp re-import

The `Mesh` class itself is deleted.

---

## Pass Changes

**`AnimationPass`:** Iterates `mRenderItems`, calls `Instance->Graph().Execute()` for any item with a non-empty graph. No longer branches on `AnimationInstance != nullptr`.

**`PhysicsPass`:** No change to shader dispatch logic. Now reads topology from `PhysicsDeformer::SimMeshAsset()` instead of `RenderItem::Geo->SimMesh*`. Reads solver state from `PhysicsDeformer` member buffers instead of `MeshAnimationResource`.

**`GBufferPass`:** Reads vertex buffer via `ri->Instance->FinalVertexBuffer()` — if the graph is non-empty, this is the deformed output; if empty (static), this is the asset's `VertexBufferGPU`. No flag checks needed.

---

## Files Changed

| File | Change |
|------|--------|
| `Render/Resources/Mesh.h/.cpp` | Replaced by `RenderMeshAsset` and `SimMeshAsset` |
| `Render/Resources/RenderItem.h/.cpp` | Slim struct; `BuildRenderItems` replaced by `AssetManager::PushRenderItems` |
| `Render/Resources/MeshAnimationResource.h/.cpp` | Deleted; state split into `SkinDeformer`, `BlendshapeDeformer`, `PhysicsDeformer` |
| `Render/Resources/Skinning.h` | `SkinningController` kept; moves into `SkinDeformer` |
| `Render/Resources/Blendshapes.h` | `BlendshapeController` kept; moves into `BlendshapeDeformer` |
| `Common/Structures.h` | `MeshGeometry` deleted; `ItemData` and `RenderItemSettings` updated |
| `Assets/AssetManager.h/.cpp` | New maps; old maps removed; `PushMesh` updated |
| `Build/PushRenderItems.cpp` | Replaced by `AssetManager::PushRenderItems` internal method |
| `Serialize/LevelReader.cpp` | Parse `animated`, `simulated`, `simulationMask` fields |
| `Levels/DemoLevel.json` | Schema updated per above |
| `Render/Passes/AnimationPass.h/.cpp` | Calls `Graph().Execute()` instead of branching on controllers |
| `Render/Passes/PhysicsPass.h/.cpp` | Reads from deformer instead of `MeshAnimationResource` + `Geo` |
| `Render/Passes/GBufferPass.h` | Uses `FinalVertexBuffer()` |

---

## What Does NOT Change

- All HLSL shaders — no changes. The buffer layouts and shader register bindings stay the same.
- All root signatures and PSOs — no changes.
- `Skeleton`, `Animation`, `TransformNode` — unchanged.
- `SkinningController`, `BlendshapeController` — unchanged structs; just owned differently.
- `FrameResource`, `PassConstants`, `ObjectConstants` — unchanged.
- `Subset` / `SubmeshGeometry` — unchanged (still used by `DrawArgs`).
- All other render passes (Shadow, Lighting, SSAO, SSGI, SSS, Radiance, Composite) — no changes.
