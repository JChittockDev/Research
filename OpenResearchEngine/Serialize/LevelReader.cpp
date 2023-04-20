#include "LevelReader.h"

using namespace nlohmann;

void LevelReader::SetStaticRenderItemData(const json& item_data, ItemData& renderItemStruct)
{
    if (item_data.contains("Position"))
    {
        std::vector<double> position = item_data["Position"];
        renderItemStruct.position = position;
    }
    if (item_data.contains("Rotation"))
    {
        std::vector<double> rotation = item_data["Rotation"];
        renderItemStruct.rotation = rotation;
    }
    if (item_data.contains("Scale"))
    {
        std::vector<double> scale = item_data["Scale"];
        renderItemStruct.scale = scale;
    }
    if (item_data.contains("Geometry"))
    {
        renderItemStruct.geometry = item_data["Geometry"];
    }
    if (item_data.contains("Mesh"))
    {
        renderItemStruct.mesh = item_data["Mesh"];
    }
    if (item_data.contains("Material"))
    {
        renderItemStruct.material = item_data["Material"];
    }
    if (item_data.contains("RenderLayer"))
    {
        renderItemStruct.render_layer = item_data["RenderLayer"];
    }
}

void LevelReader::SetSkinnedRenderItemData(const json& item_data, ItemData& renderItemStruct)
{
    if (item_data.contains("Position"))
    {
        std::vector<double> position = item_data["Position"];
        renderItemStruct.position = position;
    }
    if (item_data.contains("Rotation"))
    {
        std::vector<double> rotation = item_data["Rotation"];
        renderItemStruct.rotation = rotation;
    }
    if (item_data.contains("Scale"))
    {
        std::vector<double> scale = item_data["Scale"];
        renderItemStruct.scale = scale;
    }
    if (item_data.contains("Geometry"))
    {
        renderItemStruct.geometry = item_data["Geometry"];
    }
    if (item_data.contains("Material"))
    {
        renderItemStruct.material = item_data["Material"];
    }
    if (item_data.contains("Animation"))
    {
        renderItemStruct.animation = item_data["Animation"];
    }
    if (item_data.contains("RenderLayer"))
    {
        renderItemStruct.render_layer = item_data["RenderLayer"];
    }
}

LevelReader::LevelReader(const std::string& filename)
{
    level = std::make_unique<LevelData>();
    std::ifstream json_file(filename);
    if (json_file.is_open())
    {
        json data;
        json_file >> data;
        std::string level_name = data["Level"];
        json assets = data["Assets"];
        json render_items = assets["RenderItems"];
        json static_render_items = render_items["StaticRenderItems"];
        json skinned_render_items = render_items["SkinnedRenderItems"];
        std::unordered_map<std::string, ItemData> renderItemsDict;

        for (const auto& item : static_render_items.items()) 
        {
           ItemData renderItemData;
            std::string item_name = item.key();
            json item_data = item.value();
            SetStaticRenderItemData(item_data, renderItemData);
            renderItemsDict[item_name] = renderItemData;
        }

        for (const auto& item : skinned_render_items.items())
        {
            ItemData renderItemData;
            std::string item_name = item.key();
            json item_data = item.value();
            SetSkinnedRenderItemData(item_data, renderItemData);
            renderItemsDict[item_name] = renderItemData;
        }

        level->name = level_name;
        std::unique_ptr<LevelItemData> renderItems = std::make_unique<LevelItemData>();
        renderItems->renderItemDataDict = std::make_unique<std::unordered_map<std::string, ItemData>>(renderItemsDict);
        std::unique_ptr<LevelAssetData> assetData = std::make_unique<LevelAssetData>();
        assetData->renderItemData = std::move(renderItems);
        level->data = std::move(assetData);
    }
}
