#pragma once

#include <imgui/imgui.h>

namespace DAVA
{
class UIEvent;
}

namespace ImGui
{
void Initialize();
void OnFrameBegin();
void OnFrameEnd();
void OnInput(DAVA::UIEvent* input);
void Uninitialize();
}