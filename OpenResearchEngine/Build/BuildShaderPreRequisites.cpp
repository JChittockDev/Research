#include "../EngineApp.h"

void EngineApp::BuildShaderPreRequisites()
{
	std::string defaultShaderFile = "E:\\Personal Work\\Research\\OpenResearchEngine\\Shaders\\Default.hlsl";
	ShaderCompiler defaultShaderCompiler(defaultShaderFile);
	defaultShaderCompiler.SetMacro("NUM_DIR_LIGHTS", std::to_string(dynamicLights.DirectionalLights.size()));
	defaultShaderCompiler.SetMacro("NUM_POINT_LIGHTS", std::to_string(dynamicLights.PointLights.size()));
	defaultShaderCompiler.SetMacro("NUM_SPOT_LIGHTS", std::to_string(dynamicLights.SpotLights.size()));
	defaultShaderCompiler.SetShader(defaultShaderFile);
}