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
    #include "Concurrency/LockGuard.h"
    #include "Concurrency/AutoResetEvent.h"
    #include "Concurrency/ManualResetEvent.h"
    #include "Debug/Profiler.h"

    #include "_gl.h"

    #define RHI_GLES2__USE_CMDBUF_PACKING 1

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
    GLES2__DRAW_INSTANCED_PRIMITIVE = 43,
    GLES2__DRAW_INSTANCED_INDEXED_PRIMITIVE = 44,

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
CommandGLES2_DrawInstancedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawInstancedPrimitive, GLES2__DRAW_INSTANCED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint16 instanceCount;
    uint16 baseInstance;
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
CommandGLES2_DrawInstancedIndexedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawInstancedIndexedPrimitive, GLES2__DRAW_INSTANCED_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint32 firstVertex;
    uint32 startIndex;
    uint16 instanceCount;
    uint16 baseInstance;
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

#if RHI_GLES2__USE_CMDBUF_PACKING
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

typedef ResourcePool<CommandBufferGLES2_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPoolGLES2;
typedef ResourcePool<RenderPassGLES2_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPoolGLES2;
typedef ResourcePool<SyncObjectGLES2_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPoolGLES2;

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

static DAVA::ManualResetEvent _GLES2_RenderThreadSuspendSync(true, 0);
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

static std::vector<FrameGLES2> _GLES2_Frame;
static bool _GLES2_FrameStarted = false;
static unsigned _GLES2_FrameNumber = 1;
static DAVA::Spinlock _GLES2_FrameSync;
//static DAVA::Mutex              _FrameSync;

static DAVA::AutoResetEvent _GLES2_FramePreparedEvent(false, 400);
static DAVA::AutoResetEvent _GLES2_FrameDoneEvent(false, 400);

static void _ExecGL(GLCommand* command, uint32 cmdCount);

static Handle
gles2_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount);

    Handle handle = RenderPassPoolGLES2::Alloc();
    RenderPassGLES2_t* pass = RenderPassPoolGLES2::Get(handle);

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passConf.priority;

    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPoolGLES2::Alloc();
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(h);

#if !RHI_GLES2__USE_CMDBUF_PACKING
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
    _GLES2_FrameSync.Lock();

    if (!_GLES2_FrameStarted)
    {
        _GLES2_Frame.push_back(FrameGLES2());
        _GLES2_Frame.back().number = _GLES2_FrameNumber;
        _GLES2_Frame.back().sync = rhi::InvalidHandle;
        _GLES2_Frame.back().readyToExecute = false;

        Trace("\n\n-------------------------------\nframe %u started\n", _GLES2_FrameNumber);
        _GLES2_FrameStarted = true;
        ++_GLES2_FrameNumber;
        ProgGLES2::InvalidateAllConstBufferInstances();
    }

    _GLES2_Frame.back().pass.push_back(pass);

    _GLES2_FrameSync.Unlock();
}

static void
gles2_RenderPass_End(Handle pass)
{
}

namespace RenderPassGLES2
{
void Init(uint32 maxCount)
{
    RenderPassPoolGLES2::Reserve(maxCount);
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
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    cb->Begin();
    CommandGLES2_Begin* cmd = cb->allocCmd<CommandGLES2_Begin>();
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Begin();
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__BEGIN);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_End* cmd = cb->allocCmd<CommandGLES2_End>();
    cmd->syncObject = syncObject;
    cb->End();
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__END, syncObject);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdecl)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetPipelineState* cmd = cb->allocCmd<CommandGLES2_SetPipelineState>();
    cmd->vdecl = (uint16)vdecl;
    cmd->ps = ps;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_PIPELINE_STATE, ps, vdecl);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetCullMode* cmd = cb->allocCmd<CommandGLES2_SetCullMode>();
    cmd->mode = mode;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_CULL_MODE, mode);
#endif
}

//------------------------------------------------------------------------------

void gles2_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetScissorRect* cmd = cb->allocCmd<CommandGLES2_SetScissorRect>();
    cmd->x = rect.x;
    cmd->y = rect.y;
    cmd->width = rect.width;
    cmd->height = rect.height;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetViewport* cmd = cb->allocCmd<CommandGLES2_SetViewport>();
    cmd->x = uint16(vp.x);
    cmd->y = uint16(vp.y);
    cmd->width = uint16(vp.width);
    cmd->height = uint16(vp.height);
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetFillMode* cmd = cb->allocCmd<CommandGLES2_SetFillMode>();
    cmd->mode = mode;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_FILLMODE, mode);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
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
#if RHI_GLES2__USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
        CommandGLES2_SetVertexProgConstBuffer* cmd = cb->allocCmd<CommandGLES2_SetVertexProgConstBuffer>();
        cmd->buffer = buffer;
        cmd->bufIndex = bufIndex;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    if (tex != InvalidHandle)
    {
#if RHI_GLES2__USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
        CommandGLES2_SetVertexTexture* cmd = cb->allocCmd<CommandGLES2_SetVertexTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_VERTEX_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetIndices* cmd = cb->allocCmd<CommandGLES2_SetIndices>();
    cmd->ib = ib;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_INDICES, ib);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetQueryIndex* cmd = cb->allocCmd<CommandGLES2_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_QUERY_INDEX, objectIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetQueryBuffer* cmd = cb->allocCmd<CommandGLES2_SetQueryBuffer>();
    cmd->queryBuf = queryBuf;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_QUERY_BUFFER, queryBuf);
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
#if RHI_GLES2__USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
        CommandGLES2_SetFragmentProgConstBuffer* cmd = cb->allocCmd<CommandGLES2_SetFragmentProgConstBuffer>();
        cmd->bufIndex = bufIndex;
        cmd->buffer = buffer;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    if (tex != InvalidHandle)
    {
#if RHI_GLES2__USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
        CommandGLES2_SetFragmentTexture* cmd = cb->allocCmd<CommandGLES2_SetFragmentTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_FRAGMENT_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetDepthStencilState* cmd = cb->allocCmd<CommandGLES2_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_DEPTHSTENCIL_STATE, depthStencilState);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
// NOTE: expected to be called BEFORE SetFragmentTexture
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_SetSamplerState* cmd = cb->allocCmd<CommandGLES2_SetSamplerState>();
    cmd->samplerState = samplerState;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__SET_SAMPLER_STATE, samplerState);
#endif
}

//------------------------------------------------------------------------------

static int
_GLES2_GetDrawMode(PrimitiveType primType, uint32 primCount, unsigned* v_cnt)
{
    int mode = GL_TRIANGLES;

    switch (primType)
    {
    case PRIMITIVE_TRIANGLELIST:
        *v_cnt = primCount * 3;
        mode = GL_TRIANGLES;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        *v_cnt = 2 + primCount;
        mode = GL_TRIANGLE_STRIP;
        break;

    case PRIMITIVE_LINELIST:
        *v_cnt = primCount * 2;
        mode = GL_LINES;
        break;
    }

    return mode;
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    unsigned v_cnt = 0;
    int mode = _GLES2_GetDrawMode(type, count, &v_cnt);

#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_DrawPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__DRAW_PRIMITIVE, uint32(mode), v_cnt);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    unsigned v_cnt = 0;
    int mode = _GLES2_GetDrawMode(type, count, &v_cnt);

#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_DrawIndexedPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawIndexedPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__DRAW_INDEXED_PRIMITIVE, uint32(mode), v_cnt, firstVertex, startIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count)
{
    unsigned v_cnt = 0;
    int mode = _GLES2_GetDrawMode(type, count, &v_cnt);

#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_DrawInstancedPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawInstancedPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
    cmd->instanceCount = instCount;
    cmd->baseInstance = 0;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__DRAW_INSTANCED_PRIMITIVE, uint32(mode), instCount, v_cnt);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex, uint32 baseInstance)
{
    unsigned v_cnt = 0;
    int mode = _GLES2_GetDrawMode(type, count, &v_cnt);

#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    CommandGLES2_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<CommandGLES2_DrawInstancedIndexedPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
    cmd->instanceCount = instCount;
    cmd->baseInstance = baseInstance;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(GLES2__DRAW_INSTANCED_INDEXED_PRIMITIVE, uint32(mode), instCount, v_cnt, firstVertex, startIndex, baseInstance);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
#ifdef __DAVAENGINE_DEBUG__
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);

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

#if RHI_GLES2__USE_CMDBUF_PACKING
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
    Handle handle = SyncObjectPoolGLES2::Alloc();
    SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(handle);

    sync->is_signaled = false;
    sync->is_used = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
gles2_SyncObject_Delete(Handle obj)
{
    SyncObjectPoolGLES2::Free(obj);
}

//------------------------------------------------------------------------------

static bool
gles2_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;
    SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

CommandBufferGLES2_t::CommandBufferGLES2_t()
    : isFirstInPass(true)
    , isLastInPass(true)
    , text(nullptr)
    ,
#if RHI_GLES2__USE_CMDBUF_PACKING
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
#if RHI_GLES2__USE_CMDBUF_PACKING
    curUsedSize = 0;
#else
    _cmd.clear();
#endif
    //dbgCommandCount = 0;
}

//------------------------------------------------------------------------------

void CommandBufferGLES2_t::End()
{
#if RHI_GLES2__USE_CMDBUF_PACKING    
#else
    _cmd.push_back(EndCmd);
#endif
}


#if !RHI_GLES2__USE_CMDBUF_PACKING
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
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    Handle cur_ib = InvalidHandle;
    bool vdecl_pending = true;
    IndexSize idx_size = INDEX_SIZE_16BIT;
    unsigned tex_unit_0 = 0;
    Handle cur_query_buf = InvalidHandle;
    uint32 cur_query_i = DAVA::InvalidIndex;
    GLint def_viewport[4] = { 0, 0, 0, 0 };

    for (unsigned i = 0; i != MAX_VERTEX_STREAM_COUNT; ++i)
        cur_vb[i] = InvalidHandle;

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

#if RHI_GLES2__USE_CMDBUF_PACKING
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
        
#if RHI_GLES2__USE_CMDBUF_PACKING
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

                    TextureGLES2::SetAsRenderTarget(passCfg.colorBuffer[0].texture, passCfg.depthStencilBuffer.texture, passCfg.colorBuffer[0].textureFace, passCfg.colorBuffer[0].textureLevel);
                    def_viewport[2] = sz.dx;
                    def_viewport[3] = sz.dy;
                }
                else
                {
                    def_viewport[2] = _GLES2_DefaultFrameBuffer_Width;
                    def_viewport[3] = _GLES2_DefaultFrameBuffer_Height;
                    if (_GLES2_Binded_FrameBuffer != _GLES2_Default_FrameBuffer)
                    {
                        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer));
                        _GLES2_Binded_FrameBuffer = _GLES2_Default_FrameBuffer;
                    }
                }

                if (passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR)
                {
                    GL_CALL(glClearColor(passCfg.colorBuffer[0].clearColor[0], passCfg.colorBuffer[0].clearColor[1], passCfg.colorBuffer[0].clearColor[2], passCfg.colorBuffer[0].clearColor[3]));
                    flags |= GL_COLOR_BUFFER_BIT;
                }

                if (passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR)
                {
                        #if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
                    GL_CALL(glStencilMask(0xFFFFFFFF));
                    GL_CALL(glClearDepthf(passCfg.depthStencilBuffer.clearDepth));
                        #else
                    GL_CALL(glClearDepth(passCfg.depthStencilBuffer.clearDepth));
                    GL_CALL(glStencilMask(0xFFFFFFFF));
                    GL_CALL(glClearStencil(0));
                        #endif

                    flags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                }

                GL_CALL(glViewport(def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3]));

                if (flags)
                {
                    GL_CALL(glClear(flags));
                }
            }
        }
        break;

        case GLES2__END:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            sync = ((const CommandGLES2_End*)cmd)->syncObject;
            #else
            sync = Handle(arg[0]);
            c += 1;
            #endif

            #if defined(__DAVAENGINE_IPHONE__)
            if ((isLastInPass) && (_GLES2_Binded_FrameBuffer != _GLES2_Default_FrameBuffer)) //defualt framebuffer is discard once after frame
            {
                GLenum discards[3];
                int32 discardsCount = 0;
                if (passCfg.colorBuffer[0].storeAction == STOREACTION_NONE)
                    discards[discardsCount++] = GL_COLOR_ATTACHMENT0;
                if (passCfg.depthStencilBuffer.storeAction == STOREACTION_NONE)
                {
                    discards[discardsCount++] = GL_DEPTH_ATTACHMENT;
                    discards[discardsCount++] = GL_STENCIL_ATTACHMENT;
                }

                if (discardsCount != 0)
                    GL_CALL(glDiscardFramebufferEXT(GL_FRAMEBUFFER, discardsCount, discards));
            }

            #endif
        }
        break;

        case GLES2__SET_VERTEX_DATA:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle vb = ((const CommandGLES2_SetVertexData*)cmd)->vb;
            unsigned stream_i = ((const CommandGLES2_SetVertexData*)cmd)->streamIndex;
            #else
            Handle vb = (Handle)(arg[0]);
            unsigned stream_i = uint32(arg[1]);
            c += 2;
            #endif
            if (cur_vb[stream_i] != vb)
            {
                if (stream_i == 0)
                    VertexBufferGLES2::SetToRHI(vb);

                PipelineStateGLES2::InvalidateVattrCache();
                vdecl_pending = true;
                cur_base_vert = 0;

                StatSet::IncStat(stat_SET_VB, 1);

                cur_vb[stream_i] = vb;
            }
        }
        break;

        case GLES2__SET_INDICES:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
            cur_query_buf = ((const CommandGLES2_SetQueryBuffer*)cmd)->queryBuf;
            #else
            cur_query_buf = (Handle)(arg[0]);
            c += 1;
            #endif
        }
        break;

        case GLES2__SET_QUERY_INDEX:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            cur_query_i = ((const CommandGLES2_SetQueryIndex*)cmd)->objectIndex;
            #else
            cur_query_i = uint32(arg[0]);
            c += 1;
            #endif
        }
        break;

        case GLES2__SET_PIPELINE_STATE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
            CullMode mode = CullMode(((const CommandGLES2_SetCullMode*)cmd)->mode);
            #else
            CullMode mode = CullMode(arg[0]);
            c += 1;
            #endif

            switch (mode)
            {
            case CULL_NONE:
                GL_CALL(glDisable(GL_CULL_FACE));
                break;

            case CULL_CCW:
                GL_CALL(glEnable(GL_CULL_FACE));
                GL_CALL(glFrontFace(GL_CW));
                GL_CALL(glCullFace(GL_BACK));
                break;

            case CULL_CW:
                GL_CALL(glEnable(GL_CULL_FACE));
                GL_CALL(glFrontFace(GL_CW));
                GL_CALL(glCullFace(GL_FRONT));
                break;
            }
        }
        break;

        case GLES2__SET_SCISSOR_RECT:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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

                GL_CALL(glEnable(GL_SCISSOR_TEST));
                GL_CALL(glScissor(x, y, w, h));
            }
            else
            {
                GL_CALL(glDisable(GL_SCISSOR_TEST));
            }
        }
        break;

        case GLES2__SET_VIEWPORT:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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

                GL_CALL(glViewport(x, y, w, h));
            }
            else
            {
                GL_CALL(glViewport(def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3]));
            }
        }
        break;

        case GLES2__SET_FILLMODE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            FillMode mode = FillMode(((const CommandGLES2_SetFillMode*)cmd)->mode);
            #else
            FillMode mode = FillMode(arg[0]);
            c += 1;
            #endif

                #if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, (mode == FILLMODE_WIREFRAME) ? GL_LINE : GL_FILL));
                #endif
        }
        break;

        case GLES2__SET_DEPTHSTENCIL_STATE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle tex = ((const CommandGLES2_SetVertexTexture*)cmd)->tex;
            unsigned unit_i = ((const CommandGLES2_SetVertexTexture*)cmd)->unitIndex;
            #else
            Handle tex = (Handle)(arg[1]);
            unsigned unit_i = unsigned(arg[0]);
            c += 2;
            #endif

            TextureGLES2::SetToRHI(tex, unit_i, DAVA::InvalidIndex);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case GLES2__DRAW_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DP")}
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = ((const CommandGLES2_DrawPrimitive*)cmd)->vertexCount;
            int mode = ((const CommandGLES2_DrawPrimitive*)cmd)->mode;
            #else
            unsigned v_cnt = unsigned(arg[1]);
            int mode = int(arg[0]);
            c += 2;
            #endif

            if (last_ps != cur_ps)
            {
                PipelineStateGLES2::SetToRHI(cur_ps);
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            if (vdecl_pending)
            {
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, 0, countof(cur_vb), cur_vb);
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__DRAW_INDEXED_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DIP")}
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
                PipelineStateGLES2::SetToRHI(cur_ps);
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
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, firstVertex, countof(cur_vb), cur_vb);
                vdecl_pending = false;
                cur_base_vert = firstVertex;
            }

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            int i_sz = GL_UNSIGNED_SHORT;
            int i_off = startIndex * sizeof(uint16);

            if (idx_size == INDEX_SIZE_32BIT)
            {
                i_sz = GL_UNSIGNED_INT;
                i_off = startIndex * sizeof(uint32);
            }

            GL_CALL(glDrawElements(mode, v_cnt, i_sz, _GLES2_LastSetIndices + i_off));
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__DRAW_INSTANCED_PRIMITIVE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = ((const CommandGLES2_DrawInstancedPrimitive*)cmd)->vertexCount;
            int mode = ((const CommandGLES2_DrawInstancedPrimitive*)cmd)->mode;
            unsigned instCount = ((const CommandGLES2_DrawInstancedPrimitive*)cmd)->instanceCount;
            #else
            unsigned v_cnt = unsigned(arg[2]);
            int mode = int(arg[0]);
            unsigned instCount = int(arg[1]);
            c += 3;
            #endif

            if (last_ps != cur_ps)
            {
                PipelineStateGLES2::SetToRHI(cur_ps);
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            if (vdecl_pending)
            {
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, 0, countof(cur_vb), cur_vb);
                vdecl_pending = false;
            }

            #if defined(__DAVAENGINE_IPHONE__)
            GL_CALL(glDrawArraysInstancedEXT(mode, 0, v_cnt, instCount));
            #elif defined(__DAVAENGINE_ANDROID__)
            GL_CALL(glDrawArraysInstanced_EXT(mode, 0, v_cnt, instCount));
            #elif defined(__DAVAENGINE_MACOS__)
        GL_CALL(glDrawArraysInstancedARB(mode, 0, v_cnt, instCount));
            #else
        GL_CALL(glDrawArraysInstanced(mode, 0, v_cnt, instCount));
            #endif
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->vertexCount;
            int mode = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->mode;
            unsigned instCount = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->instanceCount;
            uint32 firstVertex = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->firstVertex;
            uint32 startIndex = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->startIndex;
            uint32 baseInst = ((const CommandGLES2_DrawInstancedIndexedPrimitive*)cmd)->baseInstance;
            #else
            unsigned v_cnt = unsigned(arg[2]);
            int mode = int(arg[0]);
            unsigned instCount = int(arg[1]);
            uint32 firstVertex = uint32(arg[3]);
            uint32 startIndex = uint32(arg[4]);
            uint32 baseInst = uint32(arg[5]);
            c += 6;
            #endif
            //{SCOPED_NAMED_TIMING("gl.DIP")}

            if (last_ps != cur_ps)
            {
                PipelineStateGLES2::SetToRHI(cur_ps);
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
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, firstVertex, countof(cur_vb), cur_vb);
                vdecl_pending = false;
                cur_base_vert = firstVertex;
            }

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::BeginQuery(cur_query_buf, cur_query_i);

            int i_sz = GL_UNSIGNED_SHORT;
            int i_off = startIndex * sizeof(uint16);

            if (idx_size == INDEX_SIZE_32BIT)
            {
                i_sz = GL_UNSIGNED_INT;
                i_off = startIndex * sizeof(uint32);
            }

            #if defined(__DAVAENGINE_IPHONE__)
            DVASSERT(baseInst == 0) // it's not supported in GLES
            GL_CALL(glDrawElementsInstancedEXT(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount));
            #elif defined(__DAVAENGINE_ANDROID__)
            DVASSERT(baseInst == 0) // it's not supported in GLES
            GL_CALL(glDrawElementsInstanced_EXT(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount));
            #elif defined(__DAVAENGINE_MACOS__)
        //            DVASSERT(baseInst == 0)
        //            GL_CALL(glDrawElementsInstancedBaseInstanceARB(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount, baseInst));
        GL_CALL(glDrawElementsInstancedBaseVertex(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount, baseInst));
            #else
        //            if( baseInst )
        GL_CALL(glDrawElementsInstancedBaseInstance(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount, baseInst));
//            else
//                GL_CALL(glDrawElementsInstanced(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount));
            #endif
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

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferGLES2::EndQuery(cur_query_buf, cur_query_i);
        }
        break;

        case GLES2__SET_MARKER:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
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
        
        #if RHI_GLES2__USE_CMDBUF_PACKING
        if (cmd->type == GLES2__END)
            break;

        c += cmd->size;
        #endif
    }

#if RHI_GLES2__USE_CMDBUF_PACKING
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
    _GLES2_FrameSync.Lock();
    for (std::vector<FrameGLES2>::iterator f = _GLES2_Frame.begin(); f != _GLES2_Frame.end();)
    {
        if (f->readyToExecute)
        {
            if (f->sync != InvalidHandle)
            {
                SyncObjectGLES2_t* s = SyncObjectPoolGLES2::Get(f->sync);
                s->is_signaled = true;
                s->is_used = true;
            }
            for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
            {
                RenderPassGLES2_t* pp = RenderPassPoolGLES2::Get(*p);

                for (std::vector<Handle>::iterator c = pp->cmdBuf.begin(), c_end = pp->cmdBuf.end(); c != c_end; ++c)
                {
                    CommandBufferGLES2_t* cc = CommandBufferPoolGLES2::Get(*c);
                    if (cc->sync != InvalidHandle)
                    {
                        SyncObjectGLES2_t* s = SyncObjectPoolGLES2::Get(cc->sync);
                        s->is_signaled = true;
                        s->is_used = true;
                    }
                    #if RHI_GLES2__USE_CMDBUF_PACKING
                    cc->curUsedSize = 0;
                    #else
                    cc->_cmd.clear();
                    #endif
                    CommandBufferPoolGLES2::Free(*c);
                }

                RenderPassPoolGLES2::Free(*p);
            }
            f = _GLES2_Frame.erase(f);
        }
        else
        {
            ++f;
        }
    }

    _GLES2_FrameSync.Unlock();
}
#endif

//------------------------------------------------------------------------------

static void
_GLES2_ExecuteQueuedCommands()
{
    Trace("rhi-gl.exec-queued-cmd\n");
    std::vector<RenderPassGLES2_t*> pass;
    std::vector<Handle> pass_h;
    unsigned frame_n = 0;
    bool do_exit = false;

    _GLES2_FrameSync.Lock();
    if (_GLES2_Frame.size())
    {
        for (std::vector<Handle>::iterator p = _GLES2_Frame.begin()->pass.begin(), p_end = _GLES2_Frame.begin()->pass.end(); p != p_end; ++p)
        {
            RenderPassGLES2_t* pp = RenderPassPoolGLES2::Get(*p);
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

        pass_h = _GLES2_Frame.begin()->pass;
        frame_n = _GLES2_Frame.begin()->number;
    }
    else
    {
        do_exit = true;
    }
    if (_GLES2_Frame.size() && (_GLES2_Frame.begin()->sync != InvalidHandle))
    {
        SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(_GLES2_Frame.begin()->sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }
    _GLES2_FrameSync.Unlock();

    if (do_exit)
        return;

    Trace("\n\n-------------------------------\nexecuting frame %u\n", frame_n);
    for (std::vector<RenderPassGLES2_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassGLES2_t* pp = *p;

        for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
        {
            Handle cb_h = pp->cmdBuf[b];
            CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cb_h);

            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
            cb->Execute();
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");

            if (cb->sync != InvalidHandle)
            {
                SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(cb->sync);

                sync->frame = frame_n;
                sync->is_signaled = false;
                sync->is_used = true;
            }

            CommandBufferPoolGLES2::Free(cb_h);
        }
    }

    _GLES2_FrameSync.Lock();
    {
        Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n", frame_n);
        _GLES2_Frame.erase(_GLES2_Frame.begin());

        for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
            RenderPassPoolGLES2::Free(*p);
    }
    _GLES2_FrameSync.Unlock();

    if (_GLES2_Context)
    {
        // take screenshot, if needed

        _GLES2_ScreenshotCallbackSync.Lock();
        if (_GLES2_PendingScreenshotCallback)
        {
            const uint32 stride = 4 * _GLES2_DefaultFrameBuffer_Width;
            uint8* rgba = new uint8[stride * _GLES2_DefaultFrameBuffer_Height];

            GLCommand cmd[] =
            {
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer } },
              { GLCommand::PIXEL_STORE_I, { GL_PACK_ALIGNMENT, 1 } },
              { GLCommand::READ_PIXELS, { 0, 0, uint64(_GLES2_DefaultFrameBuffer_Width), uint64(_GLES2_DefaultFrameBuffer_Height), GL_RGBA, GL_UNSIGNED_BYTE, (uint64)rgba } },
              { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Binded_FrameBuffer } },
            };

            _ExecGL(cmd, countof(cmd));
            for (int y = 0; y < _GLES2_DefaultFrameBuffer_Height / 2; ++y)
            {
                uint8* line1 = rgba + y * stride;
                uint8* line2 = rgba + (_GLES2_DefaultFrameBuffer_Height - y - 1) * stride;
                uint8 tmp[5 * 1024 * 4];

                DVASSERT(stride <= sizeof(tmp));
                memcpy(tmp, line1, stride);
                memcpy(line1, line2, stride);
                memcpy(line2, tmp, stride);
            }
            (*_GLES2_PendingScreenshotCallback)(_GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, rgba);
            delete[] rgba;
            _GLES2_PendingScreenshotCallback = nullptr;
        }
        _GLES2_ScreenshotCallbackSync.Unlock();

        // do swap-buffers

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

    for (SyncObjectPoolGLES2::Iterator s = SyncObjectPoolGLES2::Begin(), s_end = SyncObjectPoolGLES2::End(); s != s_end; ++s)
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

        _GLES2_FrameSync.Lock();
        {
            if (_GLES2_Frame.size())
            {
                _GLES2_Frame.back().readyToExecute = true;
                _GLES2_Frame.back().sync = sync;
                _GLES2_FrameStarted = false;
                Trace("\n\n-------------------------------\nframe %u generated\n", _GLES2_Frame.back().number);
            }

            //        _FrameStarted = false;
        }
        _GLES2_FrameSync.Unlock();

        if (!_GLES2_RenderThreadSuspended.GetRelaxed())
        {
            _GLES2_FramePreparedEvent.Signal();
        }

        unsigned frame_cnt = 0;
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
        do
        {
            _GLES2_FrameSync.Lock();
            frame_cnt = _GLES2_Frame.size();
            _GLES2_FrameSync.Unlock();

            if (frame_cnt >= _GLES2_RenderThreadFrameCount)
            {
                _GLES2_FrameDoneEvent.Wait();
            }
            //Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);
        }
        while (frame_cnt >= _GLES2_RenderThreadFrameCount);

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }
    else
    {
        if (_GLES2_Frame.size())
        {
            _GLES2_Frame.back().readyToExecute = true;
            _GLES2_Frame.back().sync = sync;
            _GLES2_FrameStarted = false;
        }

        _GLES2_ExecuteQueuedCommands();
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

    Logger::Info("[RHI] render-thread started");

    bool do_exit = false;
    while (!do_exit)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");

        {
            _GLES2_RenderThreadSuspendSync.Wait();

#if defined __DAVAENGINE_ANDROID__
            android_gl_checkSurface();
#elif defined __DAVAENGINE_IPHONE__
            ios_gl_check_layer();
#endif

            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

            // CRAP: busy-wait
            bool do_wait = true;
            while (do_wait)
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
                _GLES2_FrameSync.Lock();
                do_wait = !(_GLES2_Frame.size() && _GLES2_Frame.begin()->readyToExecute) && !_GLES2_RenderThreadSuspended.Get();
                _GLES2_FrameSync.Unlock();

                if (do_wait)
                {
                    _GLES2_FramePreparedEvent.Wait();
                }
            }
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

            if (!do_exit)
            {
                TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
                _GLES2_ExecuteQueuedCommands();
                TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
            }

            _GLES2_FrameDoneEvent.Signal();
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Logger::Info("[RHI] render-thread finished");
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

        if (_GLES2_RenderThreadSuspended.Get())
        {
            Logger::Info("RenderThreadGLES2 is suspended. Need resume it to be able to join.");
            ResumeGLES2();
        }

        _GLES2_FramePreparedEvent.Signal();

        Logger::Info("UninitializeRenderThreadGLES2 join begin");
        _GLES2_RenderThread->Join();
        Logger::Info("UninitializeRenderThreadGLES2 join end");
    }
}

//------------------------------------------------------------------------------

void SuspendGLES2()
{
    _GLES2_RenderThreadSuspended.Set(true);
    _GLES2_FramePreparedEvent.Signal(); //clear possible prepared-done sync from ExecGL
    _GLES2_FrameDoneEvent.Wait();
    _GLES2_FramePreparedEvent.Signal(); //clear possible prepared-done sync from Present
    _GLES2_FrameDoneEvent.Wait();
    _GLES2_RenderThreadSuspendSync.Reset();
    _GLES2_FramePreparedEvent.Signal(); //avoid stall
    GL_CALL(glFinish());
    Logger::Error("Render GLES Suspended");
}

//------------------------------------------------------------------------------

void ResumeGLES2()
{
    _GLES2_RenderThreadSuspendSync.Signal();
    _GLES2_RenderThreadSuspended.Set(false);
    Logger::Error("Render GLES Resumed");
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
            GL_CALL(glGenBuffers((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_BUFFER:
        {
            GL_CALL(glBindBuffer((GLenum)(arg[0]), *(GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_VERTEX_BUFFER:
        {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _GLES2_LastSetVB));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_INDEX_BUFFER:
        {
            GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _GLES2_LastSetIB));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_BUFFERS:
        {
            GL_CALL(glDeleteBuffers((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_DATA:
        {
            GL_CALL(glBufferData((GLenum)(arg[0]), (GLsizei)(arg[1]), (const void*)(arg[2]), (GLenum)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_SUBDATA:
        {
            GL_CALL(glBufferSubData((GLenum)(arg[0]), GLintptr(arg[1]), (GLsizei)(arg[2]), (const void*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_TEXTURES:
        {
            GL_CALL(glGenTextures((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_TEXTURES:
        {
            GL_CALL(glDeleteTextures((GLsizei)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::SET_ACTIVE_TEXTURE:
        {
            int t = int(arg[0]);

            if (t != _GLES2_LastActiveTexture)
            {
                GL_CALL(glActiveTexture(GLenum(t)));
                _GLES2_LastActiveTexture = t;
                cmd->status = err;
            }
        }
        break;

        case GLCommand::BIND_TEXTURE:
        {
            GL_CALL(glBindTexture((GLenum)(cmd->arg[0]), *(GLuint*)(cmd->arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::RESTORE_TEXTURE0:
        {
            GL_CALL(glBindTexture(_GLES2_LastSetTex0Target, _GLES2_LastSetTex0));
            cmd->status = err;
        }
        break;

        case GLCommand::TEX_PARAMETER_I:
        {
            GL_CALL(glTexParameteri((GLenum)(arg[0]), (GLenum)(arg[1]), (GLuint)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::TEX_IMAGE2D:
        {
            if (arg[10])
            {
                GL_CALL(glCompressedTexImage2D((GLenum)(arg[0]), (GLint)(arg[1]), (GLenum)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLsizei)(arg[8]), (const GLvoid*)(arg[9])));
            }
            else
            {
                GL_CALL(glTexImage2D((GLenum)(arg[0]), (GLint)(arg[1]), (GLint)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLenum)(arg[6]), (GLenum)(arg[7]), (const GLvoid*)(arg[9])));
            }
            cmd->status = err;
        }
        break;

        case GLCommand::GENERATE_MIPMAP:
        {
            GL_CALL(glGenerateMipmap((GLenum)(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::PIXEL_STORE_I:
        {
            GL_CALL(glPixelStorei((GLenum)(arg[0]), (GLint)(arg[1])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::READ_PIXELS:
        {
            GL_CALL(glReadPixels(GLint(arg[0]), GLint(arg[1]), GLsizei(arg[2]), GLsizei(arg[3]), GLenum(arg[4]), GLenum(arg[5]), (GLvoid*)(arg[6])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::CREATE_PROGRAM:
        {
            GL_CALL(cmd->retval = glCreateProgram());
            cmd->status = 0;
        }
        break;

        case GLCommand::CREATE_SHADER:
        {
            GL_CALL(cmd->retval = glCreateShader((GLenum)(arg[0])));
            cmd->status = 0;
        }
        break;

        case GLCommand::ATTACH_SHADER:
        {
            GL_CALL(glAttachShader(GLuint(arg[0]), GLuint(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::LINK_PROGRAM:
        {
            GL_CALL(glLinkProgram(GLuint(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::SHADER_SOURCE:
        {
            GL_CALL(glShaderSource((GLuint)(arg[0]), (GLsizei)(arg[1]), (const GLchar**)(arg[2]), (const GLint*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::COMPILE_SHADER:
        {
            GL_CALL(glCompileShader((GLuint)(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_IV:
        {
            GL_CALL(glGetShaderiv((GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_INFO_LOG:
        {
            GL_CALL(glGetShaderInfoLog((GLuint)(arg[0]), GLsizei(arg[1]), (GLsizei*)(arg[2]), (GLchar*)(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_PROGRAM_IV:
        {
            GL_CALL(glGetProgramiv((GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_ATTRIB_LOCATION:
        {
            GL_CALL(cmd->retval = glGetAttribLocation(GLuint(arg[0]), (const GLchar*)(arg[1])));
            cmd->status = 0;
        }
        break;

        case GLCommand::GET_ACTIVE_UNIFORM:
        {
            GL_CALL(glGetActiveUniform((GLuint)(arg[0]), (GLuint)(arg[1]), (GLsizei)(arg[2]), (GLsizei*)(arg[3]), (GLint*)(arg[4]), (GLenum*)(arg[5]), (GLchar*)(arg[6])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_UNIFORM_LOCATION:
        {
            GL_CALL(cmd->retval = glGetUniformLocation((GLuint)(arg[0]), (const GLchar*)(arg[1])));
            cmd->status = 0;
        }
        break;

        case GLCommand::SET_UNIFORM_1I:
        {
            GL_CALL(glUniform1i(GLint(arg[0]), GLint(arg[1])));
        }
        break;

        case GLCommand::GEN_FRAMEBUFFERS:
        {
            GL_CALL(glGenFramebuffers((GLuint)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_RENDERBUFFERS:
        {
            GL_CALL(glGenRenderbuffers((GLuint)(arg[0]), (GLuint*)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_RENDERBUFFERS:
        {
            GL_CALL(glDeleteRenderbuffers(GLsizei(arg[0]), (const GLuint*)(arg[1])));
        }
        break;

        case GLCommand::BIND_FRAMEBUFFER:
        {
            GL_CALL(glBindFramebuffer((GLenum)(arg[0]), (GLuint)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_RENDERBUFFER:
        {
            GL_CALL(glBindRenderbuffer((GLenum)(arg[0]), (GLuint)(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_TEXTURE:
        {
            GL_CALL(glFramebufferTexture2D(GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3]), GLint(arg[4])));
            cmd->status = err;
        }
        break;

        case GLCommand::RENDERBUFFER_STORAGE:
        {
            GL_CALL(glRenderbufferStorage(GLenum(arg[0]), GLenum(arg[1]), GLsizei(arg[2]), GLsizei(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_RENDERBUFFER:
        {
            GL_CALL(glFramebufferRenderbuffer(GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::FRAMEBUFFER_STATUS:
        {
            GL_CALL(cmd->retval = glCheckFramebufferStatus(GLenum(arg[0])));
            cmd->status = 0;
        }
        break;

        case GLCommand::DELETE_FRAMEBUFFERS:
        {
            GL_CALL(glDeleteFramebuffers(GLsizei(arg[0]), (const GLuint*)(arg[1])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::DRAWBUFFERS:
        {
                #if defined __DAVAENGINE_IPHONE__ || defined __DAVAENGINE_ANDROID__
                #else
            GL_CALL(glDrawBuffers(GLuint(arg[0]), (GLenum*)(arg[1])));
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
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");

        while (!scheduled)
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if (!_GLES2_PendingImmediateCmd)
            {
                _GLES2_PendingImmediateCmd = command;
                _GLES2_PendingImmediateCmdCount = cmdCount;
                scheduled = true;
            }
            _GLES2_PendingImmediateCmdSync.Unlock();
        }

        // CRAP: busy-wait
        while (!executed)
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if (!_GLES2_PendingImmediateCmd)
            {
                executed = true;
            }
            _GLES2_PendingImmediateCmdSync.Unlock();

            if (!executed)
            {
                _GLES2_FramePreparedEvent.Signal();
            }
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");
    }
}

namespace CommandBufferGLES2
{
void Init(uint32 maxCount)
{
    CommandBufferPoolGLES2::Reserve(maxCount);
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
    dispatch->impl_CommandBuffer_DrawInstancedPrimitive = &gles2_CommandBuffer_DrawInstancedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedIndexedPrimitive = &gles2_CommandBuffer_DrawInstancedIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker = &gles2_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create = &gles2_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &gles2_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &gles2_SyncObject_IsSignaled;

    dispatch->impl_Present = &gles2_Present;
}
}

} // namespace rhi
