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
    void SetStaticRenderItemData(const json& item_data, ItemData& renderItemStruct);
    void SetSkinnedRenderItemData(const json& item_data, ItemData& renderItemStruct);
};