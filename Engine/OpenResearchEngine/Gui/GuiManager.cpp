#pragma once
#include "GuiManager.h"
#include "../ImGui/imgui.h"

GuiManager::GuiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

GuiManager::~GuiManager()
{
	ImGui::DestroyContext();
}