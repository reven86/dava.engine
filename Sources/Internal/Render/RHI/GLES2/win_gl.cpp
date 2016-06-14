#include "win_gl.h"

#if defined(__DAVAENGINE_WIN32__)

#include "_gl.h"
#include "../rhi_Public.h"

void win_gl_reset(const rhi::ResetParam& param)
{
    if (wglSwapIntervalEXT != nullptr)
    {
        wglSwapIntervalEXT(param.vsyncEnabled ? 1 : 0);
    }
}

#endif
