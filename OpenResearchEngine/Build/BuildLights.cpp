#include "../EngineApp.h"

void EngineApp::SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& PointLights, const std::vector<Light>& SpotLights, std::vector<LightTransform>& LightTransforms)
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

    for (int i = 0; i < PointLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = PointLights[i];
            LightTransforms[count].ReferencePosition = PointLights[i].Position;
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

void EngineApp::BuildLights()
{
	Light light1;
    light1.Position = DirectX::XMFLOAT3(0.0, 20.0, 0.0);
	light1.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	light1.Strength = DirectX::XMFLOAT3(0.9f, 0.9f, 0.7f);
	dynamicLights.DirectionalLights.push_back(light1);

	Light light2;
    light2.Position = DirectX::XMFLOAT3(5.0, 20.0, 0.0);
	light2.Direction = DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
	light2.Strength = DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f);
	dynamicLights.DirectionalLights.push_back(light2);

    Light spotlight1;
    spotlight1.Position = DirectX::XMFLOAT3(-5.0, 13.0, 5.0);
    spotlight1.Direction = DirectX::XMFLOAT3(0.5f, -0.5f, -0.707f);
    spotlight1.Strength = DirectX::XMFLOAT3(1.65f, 1.65f, 1.65f);
    spotlight1.FalloffStart = 1.0f;
    spotlight1.FalloffEnd = 100.0f;
    spotlight1.InnerConeAngle = 0.8;
    spotlight1.OuterConeAngle = 0.9;
    dynamicLights.SpotLights.push_back(spotlight1);

    dynamicLights.LightTransforms.resize(dynamicLights.GetNumLights());
    SetLights(dynamicLights.DirectionalLights, dynamicLights.PointLights, dynamicLights.SpotLights, dynamicLights.LightTransforms);

}