/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;

    #include "_gl.h"

namespace rhi
{
//==============================================================================

struct
DepthStencilStateGLES2_t
{
    uint32 depthTestEnabled : 1;
    GLuint depthMask;
    GLenum depthFunc;

    uint32 stencilEnabled : 1;
    uint32 stencilSeparate : 1;
    struct
    {
        GLenum failOp;
        GLenum depthFailOp;
        GLenum depthStencilPassOp;
        GLenum func;
        uint32 writeMask;
        uint32 readMask;
        uint32 refValue;
    } stencilFront, stencilBack;
};

typedef ResourcePool<DepthStencilStateGLES2_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false> DepthStencilStateGLES2Pool;
RHI_IMPL_POOL(DepthStencilStateGLES2_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false);

//------------------------------------------------------------------------------

static GLenum
_CmpFunc(CmpFunc func)
{
    GLenum f = GL_ALWAYS;

    switch (func)
    {
    case CMP_NEVER:
        f = GL_NEVER;
        break;
    case CMP_LESS:
        f = GL_LESS;
        break;
    case CMP_EQUAL:
        f = GL_EQUAL;
        break;
    case CMP_LESSEQUAL:
        f = GL_LEQUAL;
        break;
    case CMP_GREATER:
        f = GL_GREATER;
        break;
    case CMP_NOTEQUAL:
        f = GL_NOTEQUAL;
        break;
    case CMP_GREATEREQUAL:
        f = GL_GEQUAL;
        break;
    case CMP_ALWAYS:
        f = GL_ALWAYS;
        break;
    }

    return f;
}

//------------------------------------------------------------------------------

static GLenum
_StencilOp(StencilOperation op)
{
    GLenum s = GL_KEEP;

    switch (op)
    {
    case STENCILOP_KEEP:
        s = GL_KEEP;
        break;
    case STENCILOP_ZERO:
        s = GL_ZERO;
        break;
    case STENCILOP_REPLACE:
        s = GL_REPLACE;
        break;
    case STENCILOP_INVERT:
        s = GL_INVERT;
        break;
    case STENCILOP_INCREMENT_CLAMP:
        s = GL_INCR;
        break;
    case STENCILOP_DECREMENT_CLAMP:
        s = GL_DECR;
        break;
    case STENCILOP_INCREMENT_WRAP:
        s = GL_INCR_WRAP;
        break;
    case STENCILOP_DECREMENT_WRAP:
        s = GL_DECR_WRAP;
        break;
    }

    return s;
}

//==============================================================================

static Handle
gles2_DepthStencilState_Create(const DepthStencilState::Descriptor& desc)
{
    Handle handle = DepthStencilStateGLES2Pool::Alloc();
    DepthStencilStateGLES2_t* state = DepthStencilStateGLES2Pool::Get(handle);

    state->depthTestEnabled = desc.depthTestEnabled;
    state->depthMask = (desc.depthWriteEnabled) ? GL_TRUE : GL_FALSE;
    state->depthFunc = _CmpFunc(CmpFunc(desc.depthFunc));

    state->stencilEnabled = desc.stencilEnabled;
    state->stencilSeparate = desc.stencilTwoSided;

    state->stencilFront.failOp = _StencilOp(StencilOperation(desc.stencilFront.failOperation));
    state->stencilFront.depthFailOp = _StencilOp(StencilOperation(desc.stencilFront.depthFailOperation));
    state->stencilFront.depthStencilPassOp = _StencilOp(StencilOperation(desc.stencilFront.depthStencilPassOperation));
    state->stencilFront.func = _CmpFunc(CmpFunc(desc.stencilFront.func));
    state->stencilFront.readMask = desc.stencilFront.readMask;
    state->stencilFront.writeMask = desc.stencilFront.writeMask;
    state->stencilFront.refValue = desc.stencilFront.refValue;

    state->stencilBack.failOp = _StencilOp(StencilOperation(desc.stencilBack.failOperation));
    state->stencilBack.depthFailOp = _StencilOp(StencilOperation(desc.stencilBack.depthFailOperation));
    state->stencilBack.depthStencilPassOp = _StencilOp(StencilOperation(desc.stencilBack.depthStencilPassOperation));
    state->stencilBack.func = _CmpFunc(CmpFunc(desc.stencilBack.func));
    state->stencilBack.readMask = desc.stencilBack.readMask;
    state->stencilBack.writeMask = desc.stencilBack.writeMask;
    state->stencilBack.refValue = desc.stencilBack.refValue;

    return handle;
}

//------------------------------------------------------------------------------

void gles2_DepthStencilState_Delete(Handle state)
{
    DepthStencilStateGLES2Pool::Free(state);
}

//==============================================================================

namespace DepthStencilStateGLES2
{
void Init(uint32 maxCount)
{
    DepthStencilStateGLES2Pool::Reserve(maxCount);
}
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_DepthStencilState_Create = &gles2_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &gles2_DepthStencilState_Delete;
}

void SetToRHI(Handle hstate)
{
    DepthStencilStateGLES2_t* state = DepthStencilStateGLES2Pool::Get(hstate);
    static int depthTestEnabled = -1;
    static int stencilEnabled = -1;
    static int depthMask = -1;
    static GLenum depthFunc = 0;

    if (state->depthTestEnabled)
    {
        if (depthTestEnabled != GL_TRUE)
        {
            glEnable(GL_DEPTH_TEST);
            depthTestEnabled = GL_TRUE;
        }

        if (depthFunc != state->depthFunc)
        {
            glDepthFunc(state->depthFunc);
            depthFunc = state->depthFunc;
        }
    }
    else
    {
        if (depthTestEnabled != GL_FALSE)
        {
            glDisable(GL_DEPTH_TEST);
            depthTestEnabled = GL_FALSE;
        }
    }

    if (depthMask != state->depthMask)
    {
        GL_CALL(glDepthMask(state->depthMask));
        depthMask = state->depthMask;
    }

    if (state->stencilEnabled)
    {
        if (stencilEnabled != GL_TRUE)
        {
            glEnable(GL_STENCIL_TEST);
            stencilEnabled = GL_TRUE;
        }

        if (state->stencilSeparate)
        {
            glStencilOpSeparate(GL_FRONT, state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp);
            glStencilFuncSeparate(GL_FRONT, state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask);
            glStencilMaskSeparate(GL_FRONT, state->stencilFront.writeMask);

            glStencilOpSeparate(GL_BACK, state->stencilBack.failOp, state->stencilBack.depthFailOp, state->stencilBack.depthStencilPassOp);
            glStencilFuncSeparate(GL_BACK, state->stencilBack.func, state->stencilBack.refValue, state->stencilBack.readMask);
            glStencilMaskSeparate(GL_BACK, state->stencilBack.writeMask);
        }
        else
        {
            glStencilOp(state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp);
            glStencilFunc(state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask);
            glStencilMask(state->stencilFront.writeMask);
        }
    }
    else
    {
        if (stencilEnabled != GL_FALSE)
        {
            glDisable(GL_STENCIL_TEST);
            stencilEnabled = GL_FALSE;
        }
    }
}
}

//==============================================================================
} // namespace rhi
