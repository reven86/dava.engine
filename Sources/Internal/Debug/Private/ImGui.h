#pragma once

#include <imgui/imgui.h>

namespace DAVA
{
class UIEvent;
}

namespace ImGui
{
void EnsureInited();
void OnFrame();
void OnInput(DAVA::UIEvent* input);
void Uninitialize();
}