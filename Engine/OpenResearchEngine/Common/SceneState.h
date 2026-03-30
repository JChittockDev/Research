// Common/SceneState.h
#pragma once
#include <vector>
#include <DirectXCollision.h>
#include "Structures.h"

struct SceneState {
    PassConstants                  mainPassCB;
    std::vector<PassConstants>     shadowPassCBs;
    std::vector<RadianceConstants> radianceCBs;
    DirectX::BoundingSphere        sceneBounds;
    DynamicLights                  lights;
    int                            clientWidth  = 0;
    int                            clientHeight = 0;
};
