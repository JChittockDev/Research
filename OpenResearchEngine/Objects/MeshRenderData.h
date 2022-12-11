#pragma once

#include "StaticMesh.h"
#include "SkinnedMesh.h"

struct MeshRenderData
{
    std::unique_ptr<StaticMesh> staticMesh = nullptr;
    std::unique_ptr<SkinnedMesh> skinnedMesh = nullptr;
};

enum MeshType : int
{
    Static,
    Skinned
};