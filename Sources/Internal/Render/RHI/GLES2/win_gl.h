#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

namespace rhi
{
struct ResetParam;
}

void win_gl_reset(const rhi::ResetParam&);

#endif
