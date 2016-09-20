#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace ProfilerOverlay
{
void Enable();
void Disable();

void OnFrameEnd(); //should be called before rhi::Present();
}
}