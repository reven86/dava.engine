#pragma once

#include "Private/imgui.h"

namespace DAVA
{
class UIEvent;
}

namespace ImGui
{
void EnsureInited();
void BeginFrame();
void OnInput(DAVA::UIEvent* input);
void Uninitialize();
}