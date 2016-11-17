#pragma once

#include <imgui/imgui.h>

namespace DAVA
{
class UIEvent;
}

namespace ImGui
{
void Initialize();
bool IsInitialized();
void OnFrameBegin();
void OnFrameEnd();
bool OnInput(DAVA::UIEvent* input);
void Uninitialize();
}
