#include "ShaderCompiler.h"

void ShaderCompiler::ReadShader(const std::string& file)
{
	std::ifstream shaderFile(file);
	if (shaderFile.is_open())
	{
		std::string shaderCode((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
		data = std::make_unique<std::string>(shaderCode);
	}
	shaderFile.close();
}

void ShaderCompiler::SetMacro(const std::string& name, const std::string& value)
{
	if (data == nullptr)
	{
		throw std::runtime_error("Shader not set, call ReadShader() before using this function");
	}

	size_t macroPos = data->find("#define " + name);
	if (macroPos != std::string::npos)
	{
		size_t endPos = data->find("\n", macroPos);
		data->replace(macroPos, endPos - macroPos, "#define " + name + " " + value);
	}
}

void ShaderCompiler::SetShader(const std::string& file)
{
	std::ofstream modifiedShaderFile(file);
	if (modifiedShaderFile.is_open())
	{
		modifiedShaderFile << *data;
	}
	modifiedShaderFile.close();
}
