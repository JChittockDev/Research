#include "../EngineApp.h"

std::unordered_map<std::string, std::string> EngineApp::GenericTextures()
{
    std::unordered_map<std::string, std::string> genericTextureDefinitions;

    genericTextureDefinitions["bricksDiffuseMap"] = "Textures/bricks2.dds";
    genericTextureDefinitions["bricksNormalMap"] = "Textures/bricks2_nmap.dds";
    genericTextureDefinitions["tileDiffuseMap"] = "Textures/tile.dds";
    genericTextureDefinitions["tileNormalMap"] = "Textures/tile_nmap.dds";
    genericTextureDefinitions["defaultDiffuseMap"] = "Textures/white1x1.dds";
    genericTextureDefinitions["defaultNormalMap"] = "Textures/default_nmap.dds";
    genericTextureDefinitions["skyCubeMap"] = "Textures/desertcube1024.dds";

    return genericTextureDefinitions;
}