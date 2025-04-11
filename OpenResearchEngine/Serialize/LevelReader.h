#pragma once

#include "nlohmann/json.hpp"
#include "../Common/Structures.h"

using namespace nlohmann;

class LevelReader
{
public:
    LevelReader(const std::string& filename);
    std::unique_ptr<LevelData> level = nullptr;

private:
    void SetRenderItemData(const json& item_data, ItemData& renderItemStruct);
    void SetPBRMaterialData(const json& item_data, PBRMaterialData& pbrMaterialStruct);
    void SetLightData(const json& light_data, LightData& lightDataStruct);
};