#pragma once

#include "../../Common/Structures.h"

class ShaderCompiler
{
public:
	ShaderCompiler() {};
	ShaderCompiler(const std::string& file) { ReadShader(file); };
	void ReadShader(const std::string& file);
	void SetMacro(const std::string& name, const std::string& value);
	void SetShader(const std::string& file);
private:
	std::unique_ptr<std::string> data = nullptr;
};