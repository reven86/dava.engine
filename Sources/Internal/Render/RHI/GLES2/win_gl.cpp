#include "win_gl.h"
#include "_gl.h"

void win_gl_reset(const rhi::ResetParam& param)
{
    if (wglSwapIntervalEXT != nullptr)
    {
        wglSwapIntervalEXT(param.vsyncEnabled ? 1 : 0);
    }
}
