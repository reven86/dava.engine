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

    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"
    #include "rhi_ProgGLES2.h"

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_RingBuffer.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"

using DAVA::Logger;
    #include "Concurrency/Thread.h"
    #include "Concurrency/Semaphore.h"
    #include "Concurrency/ConditionVariable.h"
    #include "Debug/Profiler.h"

    #include "_gl.h"

    #define RHI__GLES2_USE_CMDBUF_PACKING 1

namespace rhi
{
enum CommandGLES2Type
{
    GLES2__BEGIN = 1,
    GLES2__END = 2,

    GLES2__SET_VERTEX_DATA = 11,
    GLES2__SET_INDICES = 12,
    GLES2__SET_QUERY_BUFFER = 13,
    GLES2__SET_QUERY_INDEX = 14,

    GLES2__SET_PIPELINE_STATE = 21,
    GLES2__SET_DEPTHSTENCIL_STATE = 22,
    GLES2__SET_SAMPLER_STATE = 23,
    GLES2__SET_CULL_MODE = 24,
    GLES2__SET_SCISSOR_RECT = 25,
    GLES2__SET_VIEWPORT = 26,
    GLES2__SET_FILLMODE = 27,

    GLES2__SET_VERTEX_PROG_CONST_BUFFER = 31,
    GLES2__SET_FRAGMENT_PROG_CONST_BUFFER = 32,
    GLES2__SET_VERTEX_TEXTURE = 33,
    GLES2__SET_FRAGMENT_TEXTURE = 34,

    GLES2__DRAW_PRIMITIVE = 41,
    GLES2__DRAW_INDEXED_PRIMITIVE = 42,

    GLES2__SET_MARKER = 51,

    GLES2__NOP = 77
};

struct
RenderPassGLES2_t
{
    std::vector<Handle> cmdBuf;
    int priority;
};

#if defined(__DAVAENGINE_WIN32__)
#pragma pack(push, 1)
#endif

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#define DV_ATTR_PACKED __attribute__((packed))
#else
#define DV_ATTR_PACKED 
#endif

struct
CommandGLES2
{
    uint8 type;
    uint8 size;

    CommandGLES2(uint8 t, uint8 sz)
        : type(t)
        , size(sz)
    {
    }
} DV_ATTR_PACKED;

template <class T, CommandGLES2Type t>
struct
CommandGLES2Impl
: public CommandGLES2
{
    CommandGLES2Impl()
        : CommandGLES2(t, sizeof(T))
    {
    }
} DV_ATTR_PACKED;

struct
CommandGLES2_Begin : public CommandGLES2Impl<CommandGLES2_Begin, GLES2__BEGIN>
{
} DV_ATTR_PACKED;

struct
CommandGLES2_End : public CommandGLES2Impl<CommandGLES2_End, GLES2__END>
{
    Handle syncObject;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetVertexData : public CommandGLES2Impl<CommandGLES2_SetVertexData, GLES2__SET_VERTEX_DATA>
{
    uint16 streamIndex;
    Handle vb;
} /*DV_ATTR_PACKED*/;

struct
CommandGLES2_SetIndices : public CommandGLES2Impl<CommandGLES2_SetIndices, GLES2__SET_INDICES>
{
    Handle ib;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetQueryBuffer : public CommandGLES2Impl<CommandGLES2_SetQueryBuffer, GLES2__SET_QUERY_BUFFER>
{
    Handle queryBuf;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetQueryIndex : public CommandGLES2Impl<CommandGLES2_SetQueryIndex, GLES2__SET_QUERY_INDEX>
{
    uint32 objectIndex;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetPipelineState : public CommandGLES2Impl<CommandGLES2_SetPipelineState, GLES2__SET_PIPELINE_STATE>
{
    uint32 vdecl;
    uint32 ps;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetDepthStencilState : public CommandGLES2Impl<CommandGLES2_SetDepthStencilState, GLES2__SET_DEPTHSTENCIL_STATE>
{
    Handle depthStencilState;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetSamplerState : public CommandGLES2Impl<CommandGLES2_SetSamplerState, GLES2__SET_SAMPLER_STATE>
{
    Handle samplerState;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetCullMode : public CommandGLES2Impl<CommandGLES2_SetCullMode, GLES2__SET_CULL_MODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetScissorRect : public CommandGLES2Impl<CommandGLES2_SetScissorRect, GLES2__SET_SCISSOR_RECT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct
CommandGLES2_SetViewport : public CommandGLES2Impl<CommandGLES2_SetViewport, GLES2__SET_VIEWPORT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct
CommandGLES2_SetFillMode : public CommandGLES2Impl<CommandGLES2_SetFillMode, GLES2__SET_FILLMODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetVertexProgConstBuffer : public CommandGLES2Impl<CommandGLES2_SetVertexProgConstBuffer, GLES2__SET_VERTEX_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetFragmentProgConstBuffer : public CommandGLES2Impl<CommandGLES2_SetFragmentProgConstBuffer, GLES2__SET_FRAGMENT_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetVertexTexture : public CommandGLES2Impl<CommandGLES2_SetVertexTexture, GLES2__SET_VERTEX_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetFragmentTexture : public CommandGLES2Impl<CommandGLES2_SetFragmentTexture, GLES2__SET_FRAGMENT_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct
CommandGLES2_DrawPrimitive : public CommandGLES2Impl<CommandGLES2_DrawPrimitive, GLES2__DRAW_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
} DV_ATTR_PACKED;

struct
CommandGLES2_DrawIndexedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawIndexedPrimitive, GLES2__DRAW_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint32 firstVertex;
    uint32 startIndex;
} DV_ATTR_PACKED;

struct
CommandGLES2_SetMarker : public CommandGLES2Impl<CommandGLES2_SetMarker, GLES2__SET_MARKER>
{
    const char* text;
};


#ifdef __DAVAENGINE_WIN32__
#pragma pack(pop)
#endif

struct
CommandBufferGLES2_t
{
public:
    CommandBufferGLES2_t();
    ~CommandBufferGLES2_t();

    void Begin();
    void End();
    void Execute();

#if RHI__GLES2_USE_CMDBUF_PACKING
    template <class T>
    T* allocCmd()
    {
        if (curUsedSize + sizeof(T) >= cmdDataSize)
        {
            cmdDataSize += 4 * 1024; // CRAP: hardcoded grow-size
            cmdData = (uint8*)::realloc(cmdData, cmdDataSize);
        }

        uint8* p = cmdData + curUsedSize;
        curUsedSize += sizeof(T);

        return new ((T*)p) T();
    }

    uint8* cmdData;
    uint32 cmdDataSize;
    uint32 curUsedSize;

#else

    void Command(uint64 cmd);
    void Command(uint64 cmd, uint64 arg1);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6);

    std::vector<uint64> _cmd;

#endif

    static const uint64 EndCmd /* = 0xFFFFFFFF*/;

    RenderPassConfig passCfg;
    uint32 isFirstInPass : 1;
    uint32 isLastInPass : 1;
    uint32 usingDefaultFrameBuffer : 1;

    //    uint32              dbgCommandCount;
    RingBuffer* text;

    Handle sync;
};

struct
SyncObjectGLES2_t
{
    uint32 frame;
    uint32 is_signaled : 1;
    uint32 is_used : 1;
};

typedef ResourcePool<CommandBufferGLES2_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPool;
typedef ResourcePool<RenderPassGLES2_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPool;
typedef ResourcePool<SyncObjectGLES2_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPool;

RHI_IMPL_POOL(CommandBufferGLES2_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false);
RHI_IMPL_POOL(RenderPassGLES2_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);
RHI_IMPL_POOL(SyncObjectGLES2_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

const uint64 CommandBufferGLES2_t::EndCmd = 0xFFFFFFFF;

static bool _GLES2_CmdBufIsBeingExecuted = false;
static DAVA::Spinlock _GLES2_CmdBufIsBeingExecutedSync;

static GLCommand* _GLES2_PendingImmediateCmd = nullptr;
static uint32 _GLES2_PendingImmediateCmdCount = 0;
static DAVA::Mutex _GLES2_PendingImmediateCmdSync;

static bool _GLES2_RenderThreadExitPending = false;
static DAVA::Spinlock _GLES2_RenderThreadExitSync;
static DAVA::Semaphore _GLES2_RenderThredStartedSync(1);

static DAVA::Mutex _GLES2_RenderThreadSuspendSync;
static DAVA::Atomic<bool> _GLES2_RenderThreadSuspended(false);

static DAVA::Thread* _GLES2_RenderThread = nullptr;
static unsigned _GLES2_RenderThreadFrameCount = 0;

struct
FrameGLES2
{
    unsigned number;
    Handle sync;
    std::vector<Handle> pass;
    uint32 readyToExecute : 1;
};

static std::vector<FrameGLES2> _Frame;
static bool _FrameStarted = false;
static unsigned _FrameNumber = 1;
static DAVA::Spinlock _FrameSync;
//static DAVA::Mutex              _FrameSync;

static void _ExecGL(GLCommand* command, uint32 cmdCount);

static Handle
gles2_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount);

    Handle handle = RenderPassPool::Alloc();
    RenderPassGLES2_t* pass = RenderPassPool::Get(handle);

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passConf.priority;

    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPool::Alloc();
        CommandBufferGLES2_t* cb = CommandBufferPool::Get(h);

#if !RHI__GLES2_USE_CMDBUF_PACKING
        cb->_cmd.clear();
#endif
        cb->passCfg = passConf;
        cb->isFirstInPass = i == 0;
        cb->isLastInPass = i == cmdBufCount - 1;
        cb->usingDefaultFrameBuffer = passConf.colorBuffer[0].texture == InvalidHandle;

        pass->cmdBuf[i] = h;
        cmdBuf[i] = h;
    }

    return handle;
}

static void
gles2_RenderPass_Begin(Handle pass)
{
    _FrameSync.Lock();

    if (!_FrameStarted)
    {
        _Frame.push_back(FrameGLES2());
        _Frame.back().number = _FrameNumber;
        _Frame.back().sync = rhi::InvalidHandle;
        _Frame.back().readyToExecute = false;

        //Trace("\n\n-------------------------------\nframe %u started\n",_FrameNumber);
        _FrameStarted = true;
        ++_FrameNumber;
        ProgGLES2::InvalidateAllConstBufferInstances();
    }

    _Frame.back().pass.push_back(pass);

    _FrameSync.Unlock();
}

static void
gles2_RenderPass_End(Handle pass)
{
}

namespace RenderPassGLES2
{
void Init(uint32 maxCount)
{
    RenderPassPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &gles2_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &gles2_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &gles2_RenderPass_End;
}
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_Begin(Handle cmdBuf)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    cb->Begin();
    CommandGLES2_Begin* cmd = cb->allocCmd<CommandGLES2_Begin>();
#else
    CommandBufferPool::Get(cmdBuf)->Begin();
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__BEGIN);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_End* cmd = cb->allocCmd<CommandGLES2_End>();
    cmd->syncObject = syncObject;
    cb->End();
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__END, syncObject);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdecl)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetPipelineState* cmd = cb->allocCmd<CommandGLES2_SetPipelineState>();
    cmd->vdecl = (uint16)vdecl;
    cmd->ps = ps;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_PIPELINE_STATE, ps, vdecl);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetCullMode* cmd = cb->allocCmd<CommandGLES2_SetCullMode>();
    cmd->mode = mode;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_CULL_MODE, mode);
#endif
}

//------------------------------------------------------------------------------

void gles2_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetScissorRect* cmd = cb->allocCmd<CommandGLES2_SetScissorRect>();
    cmd->x = rect.x;
    cmd->y = rect.y;
    cmd->width = rect.width;
    cmd->height = rect.height;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetViewport* cmd = cb->allocCmd<CommandGLES2_SetViewport>();
    cmd->x = uint16(vp.x);
    cmd->y = uint16(vp.y);
    cmd->width = uint16(vp.width);
    cmd->height = uint16(vp.height);
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetFillMode* cmd = cb->allocCmd<CommandGLES2_SetFillMode>();
    cmd->mode = mode;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_FILLMODE, mode);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetVertexData* cmd = cb->allocCmd<CommandGLES2_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = uint16(streamIndex);
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_VERTEX_DATA, vb, streamIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    if (buffer != InvalidHandle)
    {
#if RHI__GLES2_USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
        CommandGLES2_SetVertexProgConstBuffer* cmd = cb->allocCmd<CommandGLES2_SetVertexProgConstBuffer>();
        cmd->buffer = buffer;
        cmd->bufIndex = bufIndex;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    if (tex != InvalidHandle)
    {
#if RHI__GLES2_USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
        CommandGLES2_SetVertexTexture* cmd = cb->allocCmd<CommandGLES2_SetVertexTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_VERTEX_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetIndices* cmd = cb->allocCmd<CommandGLES2_SetIndices>();
    cmd->ib = ib;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_INDICES, ib);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetQueryIndex* cmd = cb->allocCmd<CommandGLES2_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_QUERY_INDEX, objectIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetQueryBuffer* cmd = cb->allocCmd<CommandGLES2_SetQueryBuffer>();
    cmd->queryBuf = queryBuf;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_QUERY_BUFFER, queryBuf);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    //    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    if (buffer != InvalidHandle)
    {
#if RHI__GLES2_USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
        CommandGLES2_SetFragmentProgConstBuffer* cmd = cb->allocCmd<CommandGLES2_SetFragmentProgConstBuffer>();
        cmd->bufIndex = bufIndex;
        cmd->buffer = buffer;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    //    L_ASSERT(tex);

    if (tex != InvalidHandle)
    {
#if RHI__GLES2_USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
        CommandGLES2_SetFragmentTexture* cmd = cb->allocCmd<CommandGLES2_SetFragmentTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_FRAGMENT_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetDepthStencilState* cmd = cb->allocCmd<CommandGLES2_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_DEPTHSTENCIL_STATE, depthStencilState);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    // NOTE: expected to be called BEFORE SetFragmentTexture
#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_SetSamplerState* cmd = cb->allocCmd<CommandGLES2_SetSamplerState>();
    cmd->samplerState = samplerState;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__SET_SAMPLER_STATE, samplerState);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    unsigned v_cnt = 0;
    int mode = GL_TRIANGLES;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        v_cnt = count * 3;
        mode = GL_TRIANGLES;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        v_cnt = 2 + count;
        mode = GL_TRIANGLE_STRIP;
        break;

    case PRIMITIVE_LINELIST:
        v_cnt = count * 2;
        mode = GL_LINES;
        break;

    default:
    {
    }
    }

#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_DrawPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__DRAW_PRIMITIVE, uint32(mode), v_cnt);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    unsigned v_cnt = 0;
    int mode = GL_TRIANGLES;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        v_cnt = count * 3;
        mode = GL_TRIANGLES;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        v_cnt = 2 + count;
        mode = GL_TRIANGLE_STRIP;
        break;

    case PRIMITIVE_LINELIST:
        v_cnt = count * 2;
        mode = GL_LINES;
        break;

    default:
    {
    }
    }

#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);
    CommandGLES2_DrawIndexedPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawIndexedPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
#else
    CommandBufferPool::Get(cmdBuf)->Command(GLES2__DRAW_INDEXED_PRIMITIVE, uint32(mode), v_cnt, firstVertex, startIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
#ifdef __DAVAENGINE_DEBUG__
    CommandBufferGLES2_t* cb = CommandBufferPool::Get(cmdBuf);

    if (!cb->text)
    {
        cb->text = new RingBuffer();
        cb->text->Initialize(64 * 1024);
    }

    int len = strlen(text);
    char* txt = (char*)cb->text->Alloc(len / sizeof(float) + 2);

    memcpy(txt, text, len);
    txt[len] = '\n';
    txt[len + 1] = '\0';

#if RHI__GLES2_USE_CMDBUF_PACKING
    CommandGLES2_SetMarker* cmd = cb->allocCmd<CommandGLES2_SetMarker>();
    cmd->text = text;
#else
    cb->Command(GLES2__SET_MARKER, (uint64)(txt));
#endif

#endif
}

//------------------------------------------------------------------------------

static Handle
gles2_SyncObject_Create()
{
    Handle handle = SyncObjectPool::Alloc();
    SyncObjectGLES2_t* sync = SyncObjectPool::Get(handle);

    sync->is_signaled = false;
    sync->is_used = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
gles2_SyncObject_Delete(Handle obj)
{
    SyncObjectPool::Free(obj);
}

//------------------------------------------------------------------------------

static bool
gles2_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;
    SyncObjectGLES2_t* sync = SyncObjectPool::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

CommandBufferGLES2_t::CommandBufferGLES2_t()
    : isFirstInPass(true)
    , isLastInPass(true)
    , text(nullptr)
    ,
#if RHI__GLES2_USE_CMDBUF_PACKING
    cmdData(nullptr)
    , cmdDataSize(0)
    , curUsedSize(0)
    ,
#endif
    sync(InvalidHandle)
{
}

//------------------------------------------------------------------------------

CommandBufferGLES2_t::~CommandBufferGLES2_t()
{
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Begin()
{
#if RHI__GLES2_USE_CMDBUF_PACKING
    curUsedSize = 0;
#else
    _cmd.clear();
#endif
    //dbgCommandCount = 0;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::End()
{
#if RHI__GLES2_USE_CMDBUF_PACKING    
#else
    _cmd.push_back(EndCmd);
#endif
}


#if !RHI__GLES2_USE_CMDBUF_PACKING
//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd)
{
    _cmd.push_back(cmd);
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1)
{
    _cmd.resize(_cmd.size() + 1 + 1);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 1);

    b[0] = cmd;
    b[1] = arg1;
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1, uint64 arg2)
{
    _cmd.resize(_cmd.size() + 1 + 2);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 2);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3)
{
    _cmd.resize(_cmd.size() + 1 + 3);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 3);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4)
{
    _cmd.resize(_cmd.size() + 1 + 4);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 4);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5)
{
    _cmd.resize(_cmd.size() + 1 + 5);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 5);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
    //++dbgCommandCount;
}

//------------------------------------------------------------------------------

inline void
CommandBufferGLES2_t::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6)
{
    _cmd.resize(_cmd.size() + 1 + 6);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 6);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
    b[6] = arg6;
    //++dbgCommandCount;
}
#endif

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::Execute()
{
    //SCOPED_NAMED_TIMING("gl.exec");
    Handle cur_ps = InvalidHandle;
    uint32 cur_vdecl = VertexLayout::InvalidUID;
    uint32 cur_base_vert = 0;
    Handle last_ps = InvalidHandle;
    uint32 cur_gl_prog = 0;
    Handle vp_const[MAX_CONST_BUFFER_COUNT];
    const void* vp_const_data[MAX_CONST_BUFFER_COUNT];
    Handle fp_const[MAX_CONST_BUFFER_COUNT];
    const void* fp_const_data[MAX_CONST_BUFFER_COUNT];
    Handle cur_vb = InvalidHandle;
    Handle cur_ib = InvalidHandle;
    bool vdecl_pending = true;
    IndexSize idx_size = INDEX_SIZE_16BIT;
    unsigned tex_unit_0 = 0;
    Handle cur_query_buf = InvalidHandle;
    uint32 cur_query_i = InvalidIndex;
    GLint def_viewport[4] = { 0, 0, 0, 0 };

    for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
    {
        vp_const[i] = InvalidHandle;
        fp_const[i] = InvalidHandle;
    }
    memset(vp_const_data, 0, sizeof(vp_const_data));
    memset(fp_const_data, 0, sizeof(fp_const_data));

    int immediate_cmd_ttw = 10;

    sync = InvalidHandle;

//unsigned    cmd_cnt=0;
//unsigned    dip_cnt=0;
//unsigned    stcb_cnt=0;
//unsigned    sttx_cnt=0;

#if RHI__GLES2_USE_CMDBUF_PACKING
    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const CommandGLES2* cmd = (const CommandGLES2*)c;
#else
    for (std::vector<uint64>::const_iterator c = _cmd.begin(), c_end = _cmd.end(); c != c_end; ++c)
    {
        const uint64 cmd = *c;
        std::vector<uint64>::const_iterator arg = c + 1;

        if (cmd == EndCmd)
            break;
#endif
//++cmd_cnt;


#if RHI__GLES2_USE_CMDBUF_PACKING
        switch (cmd->type)
#else
        switch (cmd)
#endif
        {
        case GLES2__BEGIN:
        {
            GL_CALL(glFrontFace(GL_CW));
            GL_CALL(glEnable(GL_CULL_FACE));
            GL_CALL(glCullFace(GL_BACK));

            GL_CALL(glEnable(GL_DEPTH_TEST));
            GL_CALL(glDepthFunc(GL_LEQUAL));
            GL_CALL(glDepthMask(GL_TRUE));

            if (isFirstInPass)
            {
#if defined(__DAVAENGINE_IPHONE__)
                ios_gl_begin_frame();
#endif
                GLuint flags = 0;

                def_viewport[0] = 0;
                def_viewport[1] = 0;

                if (passCfg.colorBuffer[0].texture != InvalidHandle)
                {
                    Size2i sz = TextureGLES2::Size(passCfg.colorBuffer[0].texture);

                    TextureGLES2::SetAsRenderTarget(passCfg.colorBuffer[0].texture, passCfg.depthStencilBuffer.texture);
                    def_viewport[2] = sz.dx;
                    def_viewport[3] = sz.dy;
                }
                else
                {
                    GLint fbo = 0;

                    // this is SLOW AS HELL
                    //                        glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fbo );

                    if (fbo)
                    {
                        GLint type = 0;
                        GLint obj = 0;

                        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &obj);

                        if (type == GL_RENDERBUFFER)
                        {
                            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, def_viewport + 2);
                            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, def_viewport + 3);
                        }
                        else if (type == GL_TEXTURE)
                        {
                            //                                GLint   w,h;
                            //
                            //                                glGetTexParameteriv( GL_TEXTURE_2D, obj,  );
                        }
                    }
                    else
                    {
                        def_viewport[2] = _GLES2_DefaultFrameBuffer_Width;
                        def_viewport[3] = _GLES2_DefaultFrameBuffer_Height;
                        //                            glGetIntegerv( GL_VIEWPORT, def_viewport );
                    }
                }

                if (passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR)
                {
                    glClearColor(passCfg.colorBuffer[0].clearColor[0], passCfg.colorBuffer[0].clearColor[1], passCfg.colorBuffer[0].clearColor[2], passCfg.colorBuffer[0].clearColor[3]);
                    flags |= GL_COLOR_BUFFER_BIT;
                }

                if (passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR)
                {
                        #if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
                    glStencilMask(0xFFFFFFFF);
                    glClearDepthf(passCfg.depthStencilBuffer.clearDepth);
                        #else
                    glClearDepth(passCfg.depthStencilBuffer.clearDepth);
                    glStencilMask(0xFFFFFFFF);
                    glClearStencil(0);
                        #endif

                    flags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                }

                if (flags)
                {
                    glClear(flags);
                }

                glViewport(def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3]);
            }
        }
        break;

        case GLES2__END:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            sync = ((const CommandGLES2_End*)cmd)->syncObject;
            #else
            sync = Handle(arg[0]);
            c += 1;
            #endif

            if (isLastInPass)
            {
                //                    glFlush();

                if (_GLES2_Binded_FrameBuffer != _GLES2_Default_FrameBuffer)
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer);
                    _GLES2_Binded_FrameBuffer = _GLES2_Default_FrameBuffer;
                }
            }
        }
        break;

        case GLES2__SET_VERTEX_DATA:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle vb = ((const CommandGLES2_SetVertexData*)cmd)->vb;
            #else
            Handle vb = (Handle)(arg[0]);
            c += 2;
            #endif
            if (cur_vb != vb)
            {
                VertexBufferGLES2::SetToRHI(vb);
                PipelineStateGLES2::InvalidateVattrCache();
                vdecl_pending = true;
                cur_base_vert = 0;

                StatSet::IncStat(stat_SET_VB, 1);

                cur_vb = vb;
            }
        }
        break;

        case GLES2__SET_INDICES:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle ib = ((const CommandGLES2_SetIndices*)cmd)->ib;
            #else
            Handle ib = (Handle)(arg[0]);
            c += 1;
            #endif

            if (ib != cur_ib)
            {
                idx_size = IndexBufferGLES2::SetToRHI(ib);
                StatSet::IncStat(stat_SET_IB, 1);
                cur_ib = ib;
            }
        }
        break;

        case GLES2__SET_QUERY_BUFFER:
        {
            DVASSERT(cur_query_buf == InvalidHandle);
            #if RHI__GLES2_USE_CMDBUF_PACKING
            cur_query_buf = ((const CommandGLES2_SetQueryBuffer*)cmd)->queryBuf;
            #else
            cur_query_buf = (Handle)(arg[0]);
            c += 1;
            #endif
        }
        break;

        case GLES2__SET_QUERY_INDEX:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            cur_query_i = ((const CommandGLES2_SetQueryIndex*)cmd)->objectIndex;
            #else
            cur_query_i = uint32(arg[0]);
            c += 1;
            #endif
        }
        break;

        case GLES2__SET_PIPELINE_STATE:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle ps = ((const CommandGLES2_SetPipelineState*)cmd)->ps;
            uint32 vdecl = ((const CommandGLES2_SetPipelineState*)cmd)->vdecl;
            #else
            Handle ps = (Handle)arg[0];
            uint32 vdecl = (uint32)(arg[1]);
            c += 2;
            #endif

            if (cur_ps != ps || cur_vdecl != vdecl)
            {
                for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
                {
                    vp_const[i] = InvalidHandle;
                    fp_const[i] = InvalidHandle;
                }
                memset(vp_const_data, 0, sizeof(vp_const_data));
                memset(fp_const_data, 0, sizeof(fp_const_data));

                cur_ps = ps;
                cur_vdecl = vdecl;
                cur_base_vert = 0;
                last_ps = InvalidHandle;
                cur_gl_prog = PipelineStateGLES2::ProgramUid(ps);
                vdecl_pending = true;
            }

            tex_unit_0 = PipelineStateGLES2::VertexSamplerCount(ps);
        }
        break;

        case GLES2__SET_CULL_MODE:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            CullMode mode = CullMode(((const CommandGLES2_SetCullMode*)cmd)->mode);
            #else
            CullMode mode = CullMode(arg[0]);
            c += 1;
            #endif

            switch (mode)
            {
            case CULL_NONE:
                glDisable(GL_CULL_FACE);
                break;

            case CULL_CCW:
                glEnable(GL_CULL_FACE);
                glFrontFace(GL_CW);
                glCullFace(GL_BACK);
                break;

            case CULL_CW:
                glEnable(GL_CULL_FACE);
                glFrontFace(GL_CW);
                glCullFace(GL_FRONT);
                break;
            }
        }
        break;

        case GLES2__SET_SCISSOR_RECT:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            GLint x = ((const CommandGLES2_SetScissorRect*)cmd)->x;
            GLint y = ((const CommandGLES2_SetScissorRect*)cmd)->y;
            GLsizei w = ((const CommandGLES2_SetScissorRect*)cmd)->width;
            GLsizei h = ((const CommandGLES2_SetScissorRect*)cmd)->height;
            #else
            GLint x = GLint(arg[0]);
            GLint y = GLint(arg[1]);
            GLsizei w = GLsizei(arg[2]);
            GLsizei h = GLsizei(arg[3]);
            c += 4;
            #endif

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                if (usingDefaultFrameBuffer)
                    y = _GLES2_DefaultFrameBuffer_Height - y - h;

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }
        }
        break;

        case GLES2__SET_VIEWPORT:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            GLint x = ((const CommandGLES2_SetViewport*)cmd)->x;
            GLint y = ((const CommandGLES2_SetViewport*)cmd)->y;
            GLsizei w = ((const CommandGLES2_SetViewport*)cmd)->width;
            GLsizei h = ((const CommandGLES2_SetViewport*)cmd)->height;
            #else
            GLint x = GLint(arg[0]);
            GLint y = GLint(arg[1]);
            GLsizei w = GLsizei(arg[2]);
            GLsizei h = GLsizei(arg[3]);
            c += 4;
            #endif

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                if (usingDefaultFrameBuffer)
                    y = _GLES2_DefaultFrameBuffer_Height - y - h;

                glViewport(x, y, w, h);
            }
            else
            {
                glViewport(def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3]);
            }
        }
        break;

        case GLES2__SET_FILLMODE:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            FillMode mode = FillMode(((const CommandGLES2_SetFillMode*)cmd)->mode);
            #else
            FillMode mode = FillMode(arg[0]);
            c += 1;
            #endif

                #if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
            glPolygonMode(GL_FRONT_AND_BACK, (mode == FILLMODE_WIREFRAME) ? GL_LINE : GL_FILL);
                #endif
        }
        break;

        case GLES2__SET_DEPTHSTENCIL_STATE:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle state = ((const CommandGLES2_SetDepthStencilState*)cmd)->depthStencilState;
            #else
            Handle state = (Handle)(arg[0]);
            c += 1;
            #endif

            DepthStencilStateGLES2::SetToRHI(state);
        }
        break;

        case GLES2__SET_SAMPLER_STATE:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle state = ((const CommandGLES2_SetSamplerState*)cmd)->samplerState;
            #else
            Handle state = (Handle)(arg[0]);
            c += 1;
            #endif

            SamplerStateGLES2::SetToRHI(state);
            StatSet::IncStat(stat_SET_SS, 1);
        }
        break;

        case GLES2__SET_VERTEX_PROG_CONST_BUFFER:
        {
//++stcb_cnt;
            #if RHI__GLES2_USE_CMDBUF_PACKING
            unsigned buf_i = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->bufIndex;
            const void* inst = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->inst;
            Handle buf = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->buffer;
            #else
            unsigned buf_i = (unsigned)(arg[0]);
            const void* inst = (const void*)arg[2];
            Handle buf = (Handle)(arg[1]);
            c += 3;
            #endif

            vp_const[buf_i] = buf;
            vp_const_data[buf_i] = inst;
        }
        break;

        case GLES2__SET_FRAGMENT_PROG_CONST_BUFFER:
        {
//++stcb_cnt;
            #if RHI__GLES2_USE_CMDBUF_PACKING
            unsigned buf_i = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->bufIndex;
            const void* inst = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->inst;
            Handle buf = ((const CommandGLES2_SetVertexProgConstBuffer*)cmd)->buffer;
            #else
            unsigned buf_i = (unsigned)(arg[0]);
            const void* inst = (const void*)arg[2];
            Handle buf = (Handle)(arg[1]);
            c += 3;
            #endif

            fp_const[buf_i] = buf;
            fp_const_data[buf_i] = inst;
        }
        break;

        case GLES2__SET_FRAGMENT_TEXTURE:
        {
//++sttx_cnt;
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle tex = ((const CommandGLES2_SetFragmentTexture*)cmd)->tex;
            unsigned unit_i = ((const CommandGLES2_SetFragmentTexture*)cmd)->unitIndex;
            #else
            Handle tex = (Handle)(arg[1]);
            unsigned unit_i = unsigned(arg[0]);
            c += 2;
            #endif

            TextureGLES2::SetToRHI(tex, unit_i, tex_unit_0);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case GLES2__SET_VERTEX_TEXTURE:
        {
//++sttx_cnt;
            #if RHI__GLES2_USE_CMDBUF_PACKING
            Handle tex = ((const CommandGLES2_SetVertexTexture*)cmd)->tex;
            unsigned unit_i = ((const CommandGLES2_SetVertexTexture*)cmd)->unitIndex;
            #else
            Handle tex = (Handle)(arg[1]);
            unsigned unit_i = unsigned(arg[0]);
            c += 2;
            #endif

            TextureGLES2::SetToRHI(tex, unit_i, InvalidIndex);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case GLES2__DRAW_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DP")}
            #if RHI__GLES2_USE_CMDBUF_PACKING
            unsigned v_cnt = ((const CommandGLES2_DrawPrimitive*)cmd)->vertexCount;
            int mode = ((const CommandGLES2_DrawPrimitive*)cmd)->mode;
            #else
            unsigned v_cnt = unsigned(arg[1]);
            int mode = int(arg[0]);
            c += 2;
            #endif

            if (last_ps != cur_ps)
            {
                PipelineStateGLES2::SetToRHI(cur_ps, cur_vdecl);
                StatSet::IncStat(stat_SET_PS, 1);
                last_ps = cur_ps;
            }

            for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
            {
                if (vp_const[i] != InvalidHandle)
                    ConstBufferGLES2::SetToRHI(vp_const[i], cur_gl_prog, vp_const_data[i]);
            }
            for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
            {
                if (fp_const[i] != InvalidHandle)
                    ConstBufferGLES2::SetToRHI(fp_const[i], cur_gl_prog, fp_const_data[i]);
            }

            if (cur_query_i != InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            if (vdecl_pending)
            {
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl);
                vdecl_pending = false;
            }

            GL_CALL(glDrawArrays(mode, 0, v_cnt));
            StatSet::IncStat(stat_DP, 1);
            switch (mode)
            {
            case GL_TRIANGLES:
                StatSet::IncStat(stat_DTL, 1);
                break;

            case GL_TRIANGLE_STRIP:
                StatSet::IncStat(stat_DTS, 1);
                break;

            case GL_LINES:
                StatSet::IncStat(stat_DLL, 1);
                break;
            }

            if (cur_query_i != InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__DRAW_INDEXED_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DIP")}
            #if RHI__GLES2_USE_CMDBUF_PACKING
            unsigned v_cnt = ((const CommandGLES2_DrawIndexedPrimitive*)cmd)->vertexCount;
            int mode = ((const CommandGLES2_DrawIndexedPrimitive*)cmd)->mode;
            uint32 firstVertex = ((const CommandGLES2_DrawIndexedPrimitive*)cmd)->firstVertex;
            uint32 startIndex = ((const CommandGLES2_DrawIndexedPrimitive*)cmd)->startIndex;
            #else
            unsigned v_cnt = unsigned(arg[1]);
            int mode = int(arg[0]);
            uint32 firstVertex = uint32(arg[2]);
            uint32 startIndex = uint32(arg[3]);
            c += 4;
            #endif

            if (last_ps != cur_ps)
            {
                PipelineStateGLES2::SetToRHI(cur_ps, cur_vdecl);
                StatSet::IncStat(stat_SET_PS, 1);
                last_ps = cur_ps;
            }

            for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
            {
                if (vp_const[i] != InvalidHandle)
                    ConstBufferGLES2::SetToRHI(vp_const[i], cur_gl_prog, vp_const_data[i]);
            }
            for (unsigned i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
            {
                if (fp_const[i] != InvalidHandle)
                    ConstBufferGLES2::SetToRHI(fp_const[i], cur_gl_prog, fp_const_data[i]);
            }

            if (vdecl_pending || firstVertex != cur_base_vert)
            {
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, firstVertex);
                vdecl_pending = false;
                cur_base_vert = firstVertex;
            }

            if (cur_query_i != InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            int i_sz = GL_UNSIGNED_SHORT;
            int i_off = startIndex * sizeof(uint16);

            if (idx_size == INDEX_SIZE_32BIT)
            {
                i_sz = GL_UNSIGNED_INT;
                i_off = startIndex * sizeof(uint32);
            }

            GL_CALL(glDrawElements(mode, v_cnt, i_sz, (void*)((uint64)i_off)));
            StatSet::IncStat(stat_DIP, 1);
            switch (mode)
            {
            case GL_TRIANGLES:
                StatSet::IncStat(stat_DTL, 1);
                break;

            case GL_TRIANGLE_STRIP:
                StatSet::IncStat(stat_DTS, 1);
                break;

            case GL_LINES:
                StatSet::IncStat(stat_DLL, 1);
                break;
            }

            if (cur_query_i != InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__SET_MARKER:
        {
            #if RHI__GLES2_USE_CMDBUF_PACKING
            const char* text = ((const CommandGLES2_SetMarker*)cmd)->text;
            #else
            const char* text = (const char*)(arg[0]);
            c += 1;
            #endif

            Trace(text);
        }
        break;
        }

        if (--immediate_cmd_ttw <= 0)
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if (_GLES2_PendingImmediateCmd)
            {
                TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");

                _ExecGL(_GLES2_PendingImmediateCmd, _GLES2_PendingImmediateCmdCount);
                _GLES2_PendingImmediateCmd = nullptr;
                _GLES2_PendingImmediateCmdCount = 0;

                TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");
            }
            _GLES2_PendingImmediateCmdSync.Unlock();

            immediate_cmd_ttw = 10;
        }
        
        #if RHI__GLES2_USE_CMDBUF_PACKING
        if (cmd->type == GLES2__END)
            break;

        c += cmd->size;
        #endif
    }

#if RHI__GLES2_USE_CMDBUF_PACKING
//Logger::Info("exec cb  = %.2f Kb  in %u cmds (DIP=%u  STCB=%u  STTX=%u)",float(curUsedSize)/1024.0f,cmd_cnt,dip_cnt,stcb_cnt,sttx_cnt);
#else
    //Logger::Info("exec cb  = %.2f Kb  in %u cmds (%u DIPs)",float(_cmd.size()*sizeof(uint64))/1024.0f,cmd_cnt,dip_cnt);
    _cmd.clear();
#endif
}

//------------------------------------------------------------------------------

#ifdef __DAVAENGINE_ANDROID__
static void
_RejectAllFrames()
{
    _FrameSync.Lock();
    for (std::vector<FrameGLES2>::iterator f = _Frame.begin(); f != _Frame.end();)
    {
        if (f->readyToExecute)
        {
            if (f->sync != InvalidHandle)
            {
                SyncObjectGLES2_t* s = SyncObjectPool::Get(f->sync);
                s->is_signaled = true;
                s->is_used = true;
            }
            for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
            {
                RenderPassGLES2_t* pp = RenderPassPool::Get(*p);

                for (std::vector<Handle>::iterator c = pp->cmdBuf.begin(), c_end = pp->cmdBuf.end(); c != c_end; ++c)
                {
                    CommandBufferGLES2_t* cc = CommandBufferPool::Get(*c);
                    if (cc->sync != InvalidHandle)
                    {
                        SyncObjectGLES2_t* s = SyncObjectPool::Get(cc->sync);
                        s->is_signaled = true;
                        s->is_used = true;
                    }
                    #if RHI__GLES2_USE_CMDBUF_PACKING
                    cc->curUsedSize = 0;
                    #else
                    cc->_cmd.clear();
                    #endif
                    CommandBufferPool::Free(*c);
                }

                RenderPassPool::Free(*p);
            }
            f = _Frame.erase(f);
        }
        else
        {
            ++f;
        }
    }

    _FrameSync.Unlock();
}
#endif

//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommands()
{
    Trace("rhi-gl.exec-queued-cmd\n");
    std::vector<RenderPassGLES2_t*> pass;
    std::vector<Handle> pass_h;
    unsigned frame_n = 0;
    bool do_exit = false;

    _FrameSync.Lock();
    if (_Frame.size())
    {
        for (std::vector<Handle>::iterator p = _Frame.begin()->pass.begin(), p_end = _Frame.begin()->pass.end(); p != p_end; ++p)
        {
            RenderPassGLES2_t* pp = RenderPassPool::Get(*p);
            bool do_add = true;

            for (unsigned i = 0; i != pass.size(); ++i)
            {
                if (pp->priority > pass[i]->priority)
                {
                    pass.insert(pass.begin() + i, 1, pp);
                    do_add = false;
                    break;
                }
            }

            if (do_add)
                pass.push_back(pp);
        }

        pass_h = _Frame.begin()->pass;
        frame_n = _Frame.begin()->number;
    }
    else
    {
        do_exit = true;
    }
    if (_Frame.size() && (_Frame.begin()->sync != InvalidHandle))
    {
        SyncObjectGLES2_t* sync = SyncObjectPool::Get(_Frame.begin()->sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }
    _FrameSync.Unlock();

    if (do_exit)
        return;

    Trace("\n\n-------------------------------\nexecuting frame %u\n", frame_n);
    for (std::vector<RenderPassGLES2_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassGLES2_t* pp = *p;

        for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
        {
            Handle cb_h = pp->cmdBuf[b];
            CommandBufferGLES2_t* cb = CommandBufferPool::Get(cb_h);

            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
            cb->Execute();
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");

            if (cb->sync != InvalidHandle)
            {
                SyncObjectGLES2_t* sync = SyncObjectPool::Get(cb->sync);

                sync->frame = frame_n;
                sync->is_signaled = false;
                sync->is_used = true;
            }

            CommandBufferPool::Free(cb_h);
        }
    }

    _FrameSync.Lock();
    {
        Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n", frame_n);
        _Frame.erase(_Frame.begin());

        for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
            RenderPassPool::Free(*p);
    }
    _FrameSync.Unlock();

    if (_GLES2_Context)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "gl_end_frame");
        
#if defined(__DAVAENGINE_WIN32__)
        Trace("rhi-gl.swap-buffers...\n");
        SwapBuffers(_GLES2_WindowDC);
        Trace("rhi-gl.swap-buffers done\n");
#elif defined(__DAVAENGINE_MACOS__)
        macos_gl_end_frame();
#elif defined(__DAVAENGINE_IPHONE__)
        ios_gl_end_frame();
#elif defined(__DAVAENGINE_ANDROID__)

        bool success = android_gl_end_frame();
        if (!success) //'false' mean lost context, need restore resources
        {
            _RejectAllFrames();

            TextureGLES2::ReCreateAll();
            VertexBufferGLES2::ReCreateAll();
            IndexBufferGLES2::ReCreateAll();
        }

#endif

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "gl_end_frame");
    }

    // update sync-objects

    for (SyncObjectPool::Iterator s = SyncObjectPool::Begin(), s_end = SyncObjectPool::End(); s != s_end; ++s)
    {
        if (s->is_used && (frame_n - s->frame >= 2))
            s->is_signaled = true;
    }
}

//------------------------------------------------------------------------------

static void
gles2_Present(Handle sync)
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");

    if (_GLES2_RenderThreadFrameCount)
    {
        Trace("rhi-gl.present\n");

        _FrameSync.Lock();
        {
            if (_Frame.size())
            {
                _Frame.back().readyToExecute = true;
                _Frame.back().sync = sync;
                _FrameStarted = false;
                Trace("\n\n-------------------------------\nframe %u generated\n", _Frame.back().number);
            }

            //        _FrameStarted = false;
        }
        _FrameSync.Unlock();

        unsigned frame_cnt = 0;

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
        do
        {
            _FrameSync.Lock();
            frame_cnt = _Frame.size();
            _FrameSync.Unlock();

            if (frame_cnt >= _GLES2_RenderThreadFrameCount)
            {
                DAVA::Thread::Yield();
            }
            //Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);

        } while (frame_cnt >= _GLES2_RenderThreadFrameCount);

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }
    else
    {
        if (_Frame.size())
        {
            _Frame.back().readyToExecute = true;
            _Frame.back().sync = sync;
            _FrameStarted = false;
        }

        _ExecuteQueuedCommands();
    }

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
}

//------------------------------------------------------------------------------

static void
_RenderFunc(DAVA::BaseObject* obj, void*, void*)
{
    DVASSERT(_GLES2_AcquireContext);
    _GLES2_AcquireContext();

    _GLES2_RenderThredStartedSync.Post();
    Trace("RHI render-thread started\n");

    while (true)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");

        _GLES2_RenderThreadSuspendSync.Lock();

        bool do_wait = true;
        bool do_exit = false;
     
#if defined __DAVAENGINE_ANDROID__
        android_gl_checkSurface();
#endif

        // CRAP: busy-wait
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");
        do
        {
            _GLES2_RenderThreadExitSync.Lock();
            do_exit = _GLES2_RenderThreadExitPending;
            _GLES2_RenderThreadExitSync.Unlock();

            if (do_exit)
                break;

            _GLES2_PendingImmediateCmdSync.Lock();
            if (_GLES2_PendingImmediateCmd)
            {
                TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");
                _ExecGL(_GLES2_PendingImmediateCmd, _GLES2_PendingImmediateCmdCount);
                _GLES2_PendingImmediateCmd = nullptr;
                _GLES2_PendingImmediateCmdCount = 0;
                //Trace("exec-imm-cmd done\n");
                TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");
            }
            _GLES2_PendingImmediateCmdSync.Unlock();

            //            _CmdQueueSync.Lock();
            //            cnt = _RenderQueue.size();
            //            _CmdQueueSync.Unlock();
            _FrameSync.Lock();
            do_wait = !(_Frame.size() && _Frame.begin()->readyToExecute) && !_GLES2_RenderThreadSuspended.Get();
            _FrameSync.Unlock();
        } while (do_wait);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (do_exit)
            break;

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
        _ExecuteQueuedCommands();
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");

        _GLES2_RenderThreadSuspendSync.Unlock();

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Trace("RHI render-thread stopped\n");
}

void InitializeRenderThreadGLES2(uint32 frameCount)
{
    _GLES2_RenderThreadFrameCount = frameCount;

    if (_GLES2_RenderThreadFrameCount)
    {
        DVASSERT(_GLES2_ReleaseContext);
        _GLES2_ReleaseContext();

        _GLES2_RenderThread = DAVA::Thread::Create(DAVA::Message(&_RenderFunc));
        _GLES2_RenderThread->SetName("RHI.gl-render");
        _GLES2_RenderThread->Start();
        //        _GLES2_RenderThread->SetPriority(DAVA::Thread::PRIORITY_HIGH);
        _GLES2_RenderThredStartedSync.Wait();
    }
}

//------------------------------------------------------------------------------

void UninitializeRenderThreadGLES2()
{
    if (_GLES2_RenderThreadFrameCount)
    {
        _GLES2_RenderThreadExitSync.Lock();
        _GLES2_RenderThreadExitPending = true;
        _GLES2_RenderThreadExitSync.Unlock();

        _GLES2_RenderThread->Join();
    }
}

//------------------------------------------------------------------------------

void SuspendGLES2()
{
    _GLES2_RenderThreadSuspended.Set(true);
    _GLES2_RenderThreadSuspendSync.Lock();
    glFinish();
}

//------------------------------------------------------------------------------

void ResumeGLES2()
{
    _GLES2_RenderThreadSuspendSync.Unlock();
    _GLES2_RenderThreadSuspended.Set(false);
}

//------------------------------------------------------------------------------

#if 0
    
static void
_LogGLError( const char* expr, int err )
{
    Trace( "FAILED  %s (err= 0x%X) : %s\n", expr, err, GetGLErrorString(err) );
    DVASSERT(!"KABOOM!!!");
}
#endif

//------------------------------------------------------------------------------

static void
_ExecGL(GLCommand* command, uint32 cmdCount)
{
    int err = GL_NO_ERROR;

/*
    do 
    {
        err = glGetError();
    } 
    while ( err != GL_NO_ERROR );
*/

#if 0

    do 
    {
        err = glGetError();
    } 
    while ( err != GL_NO_ERROR );

    #define EXEC_GL(expr) \
    expr; \
    err = glGetError(); \
    if (err != GL_NO_ERROR) \
        _LogGLError(#expr, err); \

#else

    #define EXEC_GL(expr) expr 

#endif

    for (GLCommand *cmd = command, *cmdEnd = command + cmdCount; cmd != cmdEnd; ++cmd)
    {
        const uint64* arg = cmd->arg;

        switch (cmd->func)
        {
        case GLCommand::NOP:
        {
            // do NOTHING
        }
        break;

        case GLCommand::GEN_BUFFERS:
        {
            EXEC_GL(glGenBuffers((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_BUFFER:
        {
            EXEC_GL(glBindBuffer((GLenum)(arg[0]), *(GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_VERTEX_BUFFER:
        {
            EXEC_GL(glBindBuffer(GL_ARRAY_BUFFER, _GLES2_LastSetVB));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_INDEX_BUFFER:
        {
            EXEC_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _GLES2_LastSetIB));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_BUFFERS:
        {
            EXEC_GL(glDeleteBuffers((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_DATA:
        {
            EXEC_GL(glBufferData((GLenum)(arg[0]), (GLsizei)(arg[1]), (const void*)(arg[2]), (GLenum)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_SUBDATA:
        {
            EXEC_GL(glBufferSubData((GLenum)(arg[0]), GLintptr(arg[1]), (GLsizei)(arg[2]), (const void*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_TEXTURES:
        {
            EXEC_GL(glGenTextures((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_TEXTURES:
        {
            EXEC_GL(glDeleteTextures((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::SET_ACTIVE_TEXTURE:
        {
            int t = int(arg[0]);

            if (t != _GLES2_LastActiveTexture)
            {
                EXEC_GL(glActiveTexture(GLenum(t)));
                _GLES2_LastActiveTexture = t;
                cmd->status = err;
            }
        }
        break;

        case GLCommand::BIND_TEXTURE:
        {
            EXEC_GL(glBindTexture((GLenum)(cmd->arg[0]), *(GLuint*)(cmd->arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_TEXTURE0:
        {
            EXEC_GL(glBindTexture(_GLES2_LastSetTex0Target, _GLES2_LastSetTex0));
            cmd->status = err;
        }
        break;

        case GLCommand::TEX_PARAMETER_I:
        {
            EXEC_GL(glTexParameteri((GLenum)(arg[0]), (GLenum)(arg[1]), (GLuint)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::TEX_IMAGE2D:
        {
            if (arg[10])
            {
                EXEC_GL(glCompressedTexImage2D((GLenum)(arg[0]), (GLint)(arg[1]), (GLenum)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLsizei)(arg[8]), (const GLvoid*)(arg[9])));
            }
            else
            {
                EXEC_GL(glTexImage2D((GLenum)(arg[0]), (GLint)(arg[1]), (GLint)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLenum)(arg[6]), (GLenum)(arg[7]), (const GLvoid*)(arg[9])));
            }
            cmd->status = err;
        }
        break;

        case GLCommand::GENERATE_MIPMAP:
        {
            EXEC_GL(glGenerateMipmap((GLenum)(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::READ_PIXELS:
        {
            EXEC_GL(glReadPixels(GLint(arg[0]), GLint(arg[1]), GLsizei(arg[2]), GLsizei(arg[3]), GLenum(arg[4]), GLenum(arg[5]), (GLvoid*)(arg[6])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::CREATE_PROGRAM:
        {
            cmd->retval = glCreateProgram();
            cmd->status = 0;
        }
        break;

        case GLCommand::CREATE_SHADER:
        {
            cmd->retval = glCreateShader((GLenum)(arg[0]));
            cmd->status = 0;
        }
        break;

        case GLCommand::ATTACH_SHADER:
        {
            EXEC_GL(glAttachShader(GLuint(arg[0]), GLuint(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::LINK_PROGRAM:
        {
            EXEC_GL(glLinkProgram(GLuint(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::SHADER_SOURCE:
        {
            EXEC_GL(glShaderSource((GLuint)(arg[0]), (GLsizei)(arg[1]), (const GLchar**)(arg[2]), (const GLint*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::COMPILE_SHADER:
        {
            EXEC_GL(glCompileShader((GLuint)(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_IV:
        {
            EXEC_GL(glGetShaderiv((GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_INFO_LOG:
        {
            EXEC_GL(glGetShaderInfoLog((GLuint)(arg[0]), GLsizei(arg[1]), (GLsizei*)(arg[2]), (GLchar*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_PROGRAM_IV:
        {
            EXEC_GL(glGetProgramiv((GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_ATTRIB_LOCATION:
        {
            cmd->retval = glGetAttribLocation(GLuint(arg[0]), (const GLchar*)(arg[1]));
            cmd->status = 0;
        }
        break;

        case GLCommand::GET_ACTIVE_UNIFORM:
        {
            EXEC_GL(glGetActiveUniform((GLuint)(arg[0]), (GLuint)(arg[1]), (GLsizei)(arg[2]), (GLsizei*)(arg[3]), (GLint*)(arg[4]), (GLenum*)(arg[5]), (GLchar*)(arg[6])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_UNIFORM_LOCATION:
        {
            cmd->retval = glGetUniformLocation((GLuint)(arg[0]), (const GLchar*)(arg[1]));
            cmd->status = 0;
        }
        break;

        case GLCommand::SET_UNIFORM_1I:
        {
            EXEC_GL(glUniform1i(GLint(arg[0]), GLint(arg[1])));
        }
        break;

        case GLCommand::GEN_FRAMEBUFFERS:
        {
            EXEC_GL(glGenFramebuffers((GLuint)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_RENDERBUFFERS:
        {
            EXEC_GL(glGenRenderbuffers((GLuint)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_RENDERBUFFERS:
        {
            EXEC_GL(glDeleteRenderbuffers(GLsizei(arg[0]), (const GLuint*)(arg[1])));
        }
        break;

        case GLCommand::BIND_FRAMEBUFFER:
        {
            EXEC_GL(glBindFramebuffer((GLenum)(arg[0]), (GLuint)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_RENDERBUFFER:
        {
            EXEC_GL(glBindRenderbuffer((GLenum)(arg[0]), (GLuint)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_TEXTURE:
        {
            EXEC_GL(glFramebufferTexture2D(GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3]), GLint(arg[4])));
            cmd->status = err;
        }
        break;

        case GLCommand::RENDERBUFFER_STORAGE:
        {
            EXEC_GL(glRenderbufferStorage(GLenum(arg[0]), GLenum(arg[1]), GLsizei(arg[2]), GLsizei(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_RENDERBUFFER:
        {
            EXEC_GL(glFramebufferRenderbuffer(GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_STATUS:
        {
            cmd->retval = glCheckFramebufferStatus(GLenum(arg[0]));
            cmd->status = 0;
        }
        break;

        case GLCommand::DELETE_FRAMEBUFFERS:
        {
            EXEC_GL(glDeleteFramebuffers(GLsizei(arg[0]), (const GLuint*)(arg[1])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::DRAWBUFFERS:
        {
                #if defined __DAVAENGINE_IPHONE__ || defined __DAVAENGINE_ANDROID__
                #else
            EXEC_GL(glDrawBuffers(GLuint(arg[0]), (GLenum*)(arg[1])));
            cmd->status = err;
                #endif
        }
        break;
        }
    }
#undef EXEC_GL
}

//------------------------------------------------------------------------------

void ExecGL(GLCommand* command, uint32 cmdCount, bool force_immediate)
{
    if (force_immediate || !_GLES2_RenderThreadFrameCount)
    {
        _ExecGL(command, cmdCount);
    }
    else
    {
        bool scheduled = false;
        bool executed = false;

        // CRAP: busy-wait
        do
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if (!_GLES2_PendingImmediateCmd)
            {
                _GLES2_PendingImmediateCmd = command;
                _GLES2_PendingImmediateCmdCount = cmdCount;
                scheduled = true;
            }
            _GLES2_PendingImmediateCmdSync.Unlock();
        } while (!scheduled);

        // CRAP: busy-wait
        do
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if (!_GLES2_PendingImmediateCmd)
            {
                executed = true;
            }
            _GLES2_PendingImmediateCmdSync.Unlock();
        } while (!executed);
    }
}

namespace CommandBufferGLES2
{
void Init(uint32 maxCount)
{
    CommandBufferPool::Reserve(maxCount);
}
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_CommandBuffer_Begin = &gles2_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End = &gles2_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState = &gles2_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode = &gles2_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect = &gles2_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport = &gles2_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetFillMode = &gles2_CommandBuffer_SetFillMode;
    dispatch->impl_CommandBuffer_SetVertexData = &gles2_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer = &gles2_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture = &gles2_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices = &gles2_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer = &gles2_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex = &gles2_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &gles2_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture = &gles2_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState = &gles2_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState = &gles2_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive = &gles2_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive = &gles2_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker = &gles2_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create = &gles2_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &gles2_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &gles2_SyncObject_IsSignaled;

    dispatch->impl_Present = &gles2_Present;
}
}

} // namespace rhi
