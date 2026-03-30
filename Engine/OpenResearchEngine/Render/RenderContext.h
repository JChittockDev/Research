#pragma once
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>
#include "../Common/Structures.h"
#include "../Common/SceneState.h"

struct RenderItem;
struct FrameResource;
class  AssetManager;
struct SceneState;

// Shared per-frame state passed to every render pass.
// Passes do NOT own any of these pointers.
struct RenderContext
{
    ID3D12GraphicsCommandList*   cmdList      = nullptr;
    D3D12_VIEWPORT               viewport     = {};
    D3D12_RECT                   scissorRect  = {};
    D3D12_CPU_DESCRIPTOR_HANDLE  dsv          = {};

    // CompositePass: needs back-buffer RTV and resource for barriers
    ID3D12Resource*              backBuffer    = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE  backBufferRtv = {};

    // Scene data (read-only views — do not take ownership)
    const std::unordered_map<std::string, std::vector<std::shared_ptr<RenderItem>>>* renderItemLayers = nullptr;
    const DynamicLights*                                                              lights           = nullptr;

    // Asset and frame state access
    const AssetManager* assets     = nullptr;
    const SceneState*   sceneState = nullptr;
};
