#include "../EngineApp.h"

void EngineApp::SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms)
{
    int count = 0;
    for (int i = 0; i < DirectionalLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = DirectionalLights[i];
            LightTransforms[count].ReferencePosition = DirectionalLights[i].Position;
            count++;
        }
    }

    for (int i = 0; i < SpotLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = SpotLights[i];
            LightTransforms[count].ReferencePosition = SpotLights[i].Position;
            count++;
        }
    }
};

void EngineApp::PushLights()
{
    for (const auto& item : mLevelLights.at("DemoLevel"))
    {
        Light light;
        light.Position = DirectX::XMFLOAT3(item.second.position[0], item.second.position[1], item.second.position[2]);
        light.Direction = DirectX::XMFLOAT3(item.second.direction[0], item.second.direction[1], item.second.direction[2]);
        light.Strength = DirectX::XMFLOAT3(item.second.strength[0], item.second.strength[1], item.second.strength[2]);
        light.FalloffStart = item.second.falloff_start;
        light.FalloffEnd = item.second.falloff_end;
        light.InnerConeAngle = item.second.inner_cone_angle;
        light.OuterConeAngle = item.second.outer_cone_angle;

        if (item.second.type == "Directional")
        {
            dynamicLights.DirectionalLights.push_back(light);
        }
        else if (item.second.type == "Spot")
        {
            dynamicLights.SpotLights.push_back(light);
        }
    }

    dynamicLights.LightTransforms.resize(dynamicLights.GetNumLights());
    SetLights(dynamicLights.DirectionalLights, dynamicLights.SpotLights, dynamicLights.LightTransforms);
}