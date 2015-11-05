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

    #include "rhi_ProgGLES2.h"
    #include "../Common/rhi_Private.h"
    #include "../Common/dbg_StatSet.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_RingBuffer.h"

    #include "FileSystem/Logger.h"
using DAVA::Logger;
    #include "Debug/Profiler.h"

    #include "rhi_GLES2.h"
    #include "_gl.h"

    #include <stdio.h>
    #include <string.h>

namespace rhi
{
//==============================================================================

typedef ResourcePool<ProgGLES2::ConstBuf, RESOURCE_CONST_BUFFER, ProgGLES2::ConstBuf::Desc, false> ConstBufGLES2Pool;
RHI_IMPL_POOL_SIZE(ProgGLES2::ConstBuf, RESOURCE_CONST_BUFFER, ProgGLES2::ConstBuf::Desc, false, 12 * 1024);

static RingBuffer DefaultConstRingBuffer;
uint32 ProgGLES2::ConstBuf::CurFrame = 0;

//==============================================================================

static void
DumpShaderText(const char* code, unsigned code_sz)
{
    char src[64 * 1024];
    char* src_line[1024];
    unsigned line_cnt = 0;

    if (code_sz < sizeof(src))
    {
        memcpy(src, code, code_sz);
        src[code_sz] = '\0';
        memset(src_line, 0, sizeof(src_line));

        src_line[line_cnt++] = src;
        for (char* s = src; *s;)
        {
            if (*s == '\n')
            {
                *s = 0;
                ++s;

                while (*s && (/**s == '\n'  ||  */ *s == '\r'))
                {
                    *s = 0;
                    ++s;
                }

                if (!(*s))
                    break;

                src_line[line_cnt] = s;
                ++line_cnt;
            }
            else if (*s == '\r')
            {
                *s = ' ';
            }
            else
            {
                ++s;
            }
        }

        for (unsigned i = 0; i != line_cnt; ++i)
        {
            Logger::Info("%4u |  %s", 1 + i, src_line[i]);
        }
    }
    else
    {
        Logger::Info(code);
    }
}

//==============================================================================

ProgGLES2::ProgGLES2(ProgType t)
    : type(t)
    , prog(0)
    , shader(0)
    , texunitInited(false)
{
    for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count = 0;
    }

    memset(cbufLastBoundData, 0, sizeof(cbufLastBoundData));
}

//------------------------------------------------------------------------------

ProgGLES2::~ProgGLES2()
{
}

//------------------------------------------------------------------------------

bool ProgGLES2::Construct(const char* srcCode)
{
    bool success = false;
    int stype = (type == PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLCommand cmd1 = { GLCommand::CREATE_SHADER, { uint64(stype) } };

    ExecGL(&cmd1, 1);

    if (cmd1.retval)
    {
        unsigned s = cmd1.retval;
        int status = 0;
        char info[1024] = "";
        GLCommand cmd2[] =
        {
          { GLCommand::SHADER_SOURCE, { s, 1, uint64(&srcCode), 0 } },
          { GLCommand::COMPILE_SHADER, { s } },
          { GLCommand::GET_SHADER_IV, { s, GL_COMPILE_STATUS, uint64(&status) } },
          { GLCommand::GET_SHADER_INFO_LOG, { s, countof(info), 0, uint64(info) } }
        };

        ExecGL(cmd2, countof(cmd2));

        if (status)
        {
            shader = s;
            success = true;
        }
        else
        {
            Logger::Error("%sprog-compile failed:", (type == PROG_VERTEX) ? "v" : "f");
            Logger::Info(info);
            DumpShaderText(srcCode, strlen(srcCode));
        }

        memset(cbufLastBoundData, 0, sizeof(cbufLastBoundData));
    }

    return success;
}

//------------------------------------------------------------------------------

void ProgGLES2::Destroy()
{
}

//------------------------------------------------------------------------------

void ProgGLES2::GetProgParams(unsigned progUid)
{
#if DV_USE_UNIFORMBUFFER_OBJECT
    for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
    {
        char name[32];
        sprintf(name, "%s_Buffer%u_Block", (type == PROG_VERTEX) ? "VP" : "FP", i);
        GLuint loc = glGetUniformBlockIndex(progUid, name);

        if (loc != GL_INVALID_INDEX)
        {
            GLint sz;

            glGetActiveUniformBlockiv(progUid, loc, GL_UNIFORM_BLOCK_DATA_SIZE, &sz);
            GL_CALL(glUniformBlockBinding(progUid, loc, loc));

            cbuf[i].location = loc;
            cbuf[i].count = sz / (4 * sizeof(float));
        }
        else
        {
            cbuf[i].location = InvalidIndex;
            cbuf[i].count = 0;
        }
    }
#else

    for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count = 0;
    }

    GLint cnt = 0;
    GLCommand cmd1 = { GLCommand::GET_PROGRAM_IV, { progUid, GL_ACTIVE_UNIFORMS, uint64(&cnt) } };

    ExecGL(&cmd1, 1);

    for (unsigned u = 0; u != cnt; ++u)
    {
        char name[64];
        GLsizei length;
        GLint size;
        GLenum utype;
        GLCommand cmd2 = { GLCommand::GET_ACTIVE_UNIFORM, { progUid, u, uint64(sizeof(name) - 1), uint64(&length), uint64(&size), uint64(&utype), uint64(name) } };

        ExecGL(&cmd2, 1);

        for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
        {
            char n[16];
            sprintf(n, "%s_Buffer%u[0]", (type == PROG_VERTEX) ? "VP" : "FP", i);

            if (!strcmp(name, n))
            {
                int loc;
                GLCommand cmd3 = { GLCommand::GET_UNIFORM_LOCATION, { progUid, uint64(name) } };

                ExecGL(&cmd3, 1);
                loc = cmd3.retval;

                if (loc != -1)
                {
                    cbuf[i].location = loc;
                    cbuf[i].count = size;
                    break;
                }
            }
        }
    }
#endif // DV_USE_UNIFORMBUFFER_OBJECT

    // get texture location
    {
        char tname[countof(texunitLoc)][32];
        GLCommand cmd[countof(texunitLoc)];

        for (unsigned i = 0; i != countof(texunitLoc); ++i)
        {
            if (type == PROG_FRAGMENT)
                Snprintf(tname[i], countof(tname[i]), "FragmentTexture%u", i);
            else if (type == PROG_VERTEX)
                Snprintf(tname[i], countof(tname[i]), "VertexTexture%u", i);

            cmd[i].func = GLCommand::GET_UNIFORM_LOCATION;
            cmd[i].arg[0] = progUid;
            cmd[i].arg[1] = uint64(tname[i]);
        }

        ExecGL(cmd, countof(cmd));
        texunitCount = 0;

        for (unsigned i = 0; i != countof(texunitLoc); ++i)
        {
            int loc = cmd[i].retval;

            texunitLoc[i] = (loc != -1) ? loc : InvalidIndex;

            if (loc != -1)
                ++texunitCount;
        }
    }

    prog = progUid;
    texunitInited = false;
}

//------------------------------------------------------------------------------

unsigned
ProgGLES2::SamplerCount() const
{
    return texunitCount;
}

//------------------------------------------------------------------------------

unsigned
ProgGLES2::ConstBufferCount() const
{
    return countof(cbuf);
}

//------------------------------------------------------------------------------

Handle
ProgGLES2::InstanceConstBuffer(unsigned bufIndex) const
{
    Handle handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
    DVASSERT(prog != 0)
    //    DVASSERT(cbuf[bufIndex].location != InvalidIndex);

    if (bufIndex < countof(cbuf) && cbuf[bufIndex].location != InvalidIndex)
    {
        handle = ConstBufGLES2Pool::Alloc();

        ConstBuf* cb = ConstBufGLES2Pool::Get(handle);

        if (!cb->Construct(prog, (void**)(&(cbufLastBoundData[bufIndex])), cbuf[bufIndex].location, cbuf[bufIndex].count))
        {
            ConstBufGLES2Pool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

void ProgGLES2::SetupTextureUnits(unsigned baseUnit) const
{
    //    GLCommand   cmd[countof(texunitLoc)];
    //    uint32      cnt = 0;

    for (unsigned i = 0; i != countof(texunitLoc); ++i)
    {
        if (texunitLoc[i] != -1)
        {
            //{SCOPED_NAMED_TIMING("gl-Uniform1i")}
            glUniform1i(texunitLoc[i], baseUnit + i);
            //            cmd[cnt].func   = GLCommand::SET_UNIFORM_1I;
            //            cmd[cnt].arg[0] = texunitLoc[i];
            //            cmd[cnt].arg[1] = baseUnit + i;
            //
            //            ++cnt;
        }
    }
    //    ExecGL( cmd, cnt );
}

//------------------------------------------------------------------------------

bool ProgGLES2::ConstBuf::Construct(uint32 prog, void** lastBoundData, unsigned loc, unsigned cnt)
{
    bool success = true;

    glProg = prog;
    location = (uint16)loc;
    count = (uint16)cnt;
    data = (float*)(::malloc(cnt * 4 * sizeof(float)));
    inst = nullptr;
    lastInst = lastBoundData;
    *lastInst = nullptr;
    frame = 0;

    return success;
}

//------------------------------------------------------------------------------

void ProgGLES2::ConstBuf::Destroy()
{
    if (data)
    {
        ::free(data);

        data = nullptr;
        inst = nullptr;
        lastInst = nullptr;
        location = -1;
        count = 0;
    }
}

//------------------------------------------------------------------------------

unsigned
ProgGLES2::ConstBuf::ConstCount() const
{
    return count;
}

//------------------------------------------------------------------------------

bool ProgGLES2::ConstBuf::SetConst(unsigned const_i, unsigned const_count, const float* cdata)
{
    bool success = false;

    if (const_i + const_count <= count)
    {
        memcpy(data + const_i * 4, cdata, const_count * 4 * sizeof(float));
        inst = nullptr;
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool ProgGLES2::ConstBuf::SetConst(unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count)
{
    bool success = false;

    if (const_i <= count && const_sub_i < 4)
    {
        memcpy(data + const_i * 4 + const_sub_i, cdata, data_count * sizeof(float));
        inst = nullptr;
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

const void*
ProgGLES2::ConstBuf::Instance() const
{
    if (frame != CurFrame)
    {
        inst = nullptr;
        *lastInst = nullptr;
    }

    if (!inst)
    {
        //SCOPED_NAMED_TIMING("gl.cb-inst");
        inst = DefaultConstRingBuffer.Alloc(count * 4);
        memcpy(inst, data, 4 * count * sizeof(float));
        frame = CurFrame;
    }

    return inst;
}

//------------------------------------------------------------------------------

void ProgGLES2::ConstBuf::SetToRHI(uint32 progUid, const void* instData) const
{
    DVASSERT(progUid == glProg);
    if (instData != *lastInst)
    {
        //SCOPED_NAMED_TIMING("gl-Uniform4fv");
        GL_CALL(glUniform4fv(location, count, (GLfloat*)instData));
        *lastInst = (void*)(instData);
    }

    StatSet::IncStat(stat_SET_CB, 1);
}

//------------------------------------------------------------------------------

void ProgGLES2::ConstBuf::InvalidateInstance()
{
    inst = nullptr;
    *lastInst = nullptr;
}

//------------------------------------------------------------------------------

void ProgGLES2::ConstBuf::AdvanceFrame()
{
    ++CurFrame;
}

//------------------------------------------------------------------------------

unsigned
ProgGLES2::ShaderUid() const
{
    return shader;
}

//------------------------------------------------------------------------------

void ProgGLES2::InvalidateAllConstBufferInstances()
{
    ConstBuf::AdvanceFrame();
    DefaultConstRingBuffer.Reset();
    /*
    for( ConstBufGLES2Pool::Iterator b=ConstBufGLES2Pool::Begin(),b_end=ConstBufGLES2Pool::End(); b!=b_end; ++b )
    {
        b->InvalidateInstance();
    }
*/
}

//------------------------------------------------------------------------------

static unsigned
gles2_ConstBuffer_ConstCount(Handle cb)
{
    const ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    return self->ConstCount();
}

//------------------------------------------------------------------------------

static bool
gles2_ConstBuffer_SetConst(Handle cb, unsigned const_i, unsigned const_count, const float* data)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    //Logger::Info( "  upd-cb %u", unsigned(RHI_HANDLE_INDEX(cb)) );
    return self->SetConst(const_i, const_count, data);
}

//------------------------------------------------------------------------------

static bool
gles2_ConstBuffer_SetConst1(Handle cb, unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    //Logger::Info( "  upd-cb %u", unsigned(RHI_HANDLE_INDEX(cb)) );
    return self->SetConst(const_i, const_sub_i, data, dataCount);
}

//------------------------------------------------------------------------------

static void
gles2_ConstBuffer_Delete(Handle cb)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    self->Destroy();
    ConstBufGLES2Pool::Free(cb);
}

//------------------------------------------------------------------------------

namespace ConstBufferGLES2
{
void Init(uint32 maxCount)
{
    ConstBufGLES2Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_ConstBuffer_SetConst = &gles2_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv = &gles2_ConstBuffer_SetConst1;
    dispatch->impl_ConstBuffer_ConstCount = &gles2_ConstBuffer_ConstCount;
    dispatch->impl_ConstBuffer_Delete = &gles2_ConstBuffer_Delete;
}

void InitializeRingBuffer(uint32 size)
{
    DefaultConstRingBuffer.Initialize(size);
}

void SetToRHI(const Handle cb, uint32 progUid, const void* instData)
{
    const ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    //Logger::Info( "  set-cb %u  inst= %p", unsigned(RHI_HANDLE_INDEX(cb)), instData );
    self->SetToRHI(progUid, instData);
}

const void*
Instance(Handle cb)
{
    const ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    return self->Instance();
}
}

//==============================================================================
} // namespace rhi
