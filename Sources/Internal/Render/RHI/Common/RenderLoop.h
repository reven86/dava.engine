#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace RenderLoop
{
void Present(Handle syncHandle); // called from main thread

void InitializeRenderLoop(uint32 frameCount);
void UninitializeRenderLoop();

void SuspendRender();
void ResumeRender();
}
}