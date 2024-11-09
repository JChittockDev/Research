#include "LevelReader.h"

using namespace nlohmann;


void LevelReader::SetRenderItemData(const json& item_data, ItemData& renderItemStruct)
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
    if (item_data.contains("Animation"))
    {
        renderItemStruct.animation = item_data["Animation"];
    }
    if (item_data.contains("Deformable"))
    {
        renderItemStruct.deformable = item_data["Deformable"];
    }
    else
    {
        renderItemStruct.deformable = false;
    }
    if (item_data.contains("RenderLayer"))
    {
        renderItemStruct.render_layer = item_data["RenderLayer"];
    }
    if (item_data.contains("Settings"))
    {
        for (auto& riSettings : item_data["Settings"].items())
        {
            if (riSettings.value().contains("Material"))
            {
                renderItemStruct.settings[riSettings.key()].Material = riSettings.value()["Material"];
            }
            else
            {
                renderItemStruct.settings[riSettings.key()].Material = "default";
            }
            if (riSettings.value().contains("Simulation"))
            {
                renderItemStruct.settings[riSettings.key()].Simulation = riSettings.value()["Simulation"];
            }
            else
            {
                renderItemStruct.settings[riSettings.key()].Simulation = false;
            }
            if (riSettings.value().contains("SimulationMask"))
            {
                renderItemStruct.settings[riSettings.key()].SimulationMask = riSettings.value()["SimulationMask"];
            }
            else
            {
                renderItemStruct.settings[riSettings.key()].SimulationMask = "none";
            }
        }
    }
}


void LevelReader::SetPBRMaterialData(const json& item_data, PBRMaterialData& pbrMaterialStruct)
{
    if (item_data.contains("Diffuse"))
    {
        if (item_data["Diffuse"] != "")
        {
            pbrMaterialStruct.diffuse_tex_path = item_data["Diffuse"];
        }
    }
    if (item_data.contains("Normal"))
    {
        if (item_data["Normal"] != "")
        {
            pbrMaterialStruct.normal_tex_path = item_data["Normal"];
        }
    }
    if (item_data.contains("Roughness"))
    {
        if (item_data["Roughness"] != "")
        {
            pbrMaterialStruct.roughness_tex_path = item_data["Roughness"];
        }
    }
    if (item_data.contains("Metalness"))
    {
        if (item_data["Metalness"] != "")
        {
            pbrMaterialStruct.metalness_tex_path = item_data["Metalness"];
        }
    }
    if (item_data.contains("Specular"))
    {
        if (item_data["Specular"] != "")
        {
            pbrMaterialStruct.specular_tex_path = item_data["Specular"];
        }
    }
    if (item_data.contains("Height"))
    {
        if (item_data["Height"] != "")
        {
            pbrMaterialStruct.height_tex_path = item_data["Height"];
        }
    }
    if (item_data.contains("Opacity"))
    {
        if (item_data["Opacity"] != "")
        {
            pbrMaterialStruct.opacity_tex_path = item_data["Opacity"];
        }
    }
    if (item_data.contains("Occlusion"))
    {
        if (item_data["Occlusion"] != "")
        {
            pbrMaterialStruct.occlusion_tex_path = item_data["Occlusion"];
        }
    }
    if (item_data.contains("Refraction"))
    {
        if (item_data["Refraction"] != "")
        {
            pbrMaterialStruct.refraction_tex_path = item_data["Refraction"];
        }
    }
    if (item_data.contains("Emissive"))
    {
        if (item_data["Emissive"] != "")
        {
            pbrMaterialStruct.emissive_tex_path = item_data["Emissive"];
        }
    }
    if (item_data.contains("Subsurface"))
    {
        if (item_data["Subsurface"] != "")
        {
            pbrMaterialStruct.subsurface_tex_path = item_data["Subsurface"];
        }
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
        json materials = assets["Materials"];
        json render_items = assets["RenderItems"];
        std::unordered_map<std::string, ItemData> renderItemsDict;
        std::unordered_map<std::string, PBRMaterialData> pbrMaterialDict;

        for (const auto& material : materials.items())
        {
            PBRMaterialData materialData;
            std::string mat_name = material.key();
            json mat_data = material.value();
            SetPBRMaterialData(mat_data, materialData);
            pbrMaterialDict[mat_name] = materialData;
        }

        for (const auto& item : render_items.items())
        {
           ItemData renderItemData;
            std::string item_name = item.key();
            json item_data = item.value();
            SetRenderItemData(item_data, renderItemData);
            renderItemsDict[item_name] = renderItemData;
        }

        level->name = level_name;
        std::unique_ptr<LevelItemData> renderItems = std::make_unique<LevelItemData>();
        renderItems->renderItemDataDict = std::make_unique<std::unordered_map<std::string, ItemData>>(renderItemsDict);

        std::unique_ptr<LevelMaterialData> levelMaterials = std::make_unique<LevelMaterialData>();
        levelMaterials->pbrMaterialDataDict = std::make_unique<std::unordered_map<std::string, PBRMaterialData>>(pbrMaterialDict);

        std::unique_ptr<LevelAssetData> assetData = std::make_unique<LevelAssetData>();
        assetData->renderItemData = std::move(renderItems);
        level->data = std::move(assetData);
        level->materialData = std::move(levelMaterials);
    }
}
