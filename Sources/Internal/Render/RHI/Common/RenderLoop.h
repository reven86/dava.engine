#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace RenderLoop
{
void Present(Handle syncHandle); // called from main thread
}
}