#include "../EngineApp.h"

void EngineApp::SetLights(const std::vector<Light>& DirectionalLights, const std::vector<Light>& PointLights, const std::vector<Light>& SpotLights)
{
    int count = 0;
    for (int i = 0; i < DirectionalLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = DirectionalLights[i];
            count++;
        }
    }

    for (int i = 0; i < PointLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = PointLights[i];
            count++;
        }
    }

    for (int i = 0; i < SpotLights.size(); i++)
    {
        if (count < MaxLights)
        {
            mMainPassCB.Lights[count] = SpotLights[i];
            count++;
        }
    }
};

void EngineApp::BuildLights()
{
	Light light1;
	light1.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	light1.Strength = DirectX::XMFLOAT3(0.9f, 0.9f, 0.7f);
	dynamicLights.DirectionalLights.push_back(light1);

	Light light2;
	light2.Direction = DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
	light2.Strength = DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f);
	dynamicLights.DirectionalLights.push_back(light2);

	Light light3;
	light3.Direction = DirectX::XMFLOAT3(0.0f, -0.707f, -0.707f);
	light3.Strength = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
	dynamicLights.DirectionalLights.push_back(light3);

    SetLights(dynamicLights.DirectionalLights, dynamicLights.PointLights, dynamicLights.SpotLights);
}