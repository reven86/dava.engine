#include "../Common/rhi_Pool.h"
#include "rhi_GLES2.h"
#include "rhi_ProgGLES2.h"

#include "../Common/rhi_Private.h"
#include "../Common/rhi_RingBuffer.h"
#include "../Common/dbg_StatSet.h"
#include "../Common/CommonImpl.h"
#include "../Common/SoftwareCommandBuffer.h"
#include "../Common/RenderLoop.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

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
struct RenderPassGLES2_t
{
    std::vector<Handle> cmdBuf;
    int priority;
};

#if RHI_GLES2__USE_CMDBUF_PACKING
typedef SoftwareCommandBuffer SoftwareCommandBufferType;
#else
typedef SoftwareCommandBufferUnpacked SoftwareCommandBufferType;
#endif

struct CommandBufferGLES2_t : public SoftwareCommandBufferType
{
public:
    CommandBufferGLES2_t();
    ~CommandBufferGLES2_t();
    void Execute();


    RenderPassConfig passCfg;
    uint32 isFirstInPass : 1;
    uint32 isLastInPass : 1;
    uint32 usingDefaultFrameBuffer : 1;
    Handle sync;
};

struct SyncObjectGLES2_t
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

static DAVA::Mutex _GLES2_SyncObjectsSync;

static Handle gles2_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
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

static void gles2_RenderPass_Begin(Handle pass)
{
}

static void gles2_RenderPass_End(Handle pass)
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

static void gles2_CommandBuffer_Begin(Handle cmdBuf)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    cb->Begin();
    SWCommand_Begin* cmd = cb->allocCmd<SWCommand_Begin>();
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Begin();
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_BEGIN);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_End* cmd = cb->allocCmd<SWCommand_End>();
    cmd->syncObject = syncObject;
    cb->End();
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_END, syncObject);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdecl)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetPipelineState* cmd = cb->allocCmd<SWCommand_SetPipelineState>();
    cmd->vdecl = uint16(vdecl);
    cmd->ps = ps;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_PIPELINE_STATE, ps, vdecl);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetCullMode* cmd = cb->allocCmd<SWCommand_SetCullMode>();
    cmd->mode = mode;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_CULL_MODE, mode);
#endif
}

//------------------------------------------------------------------------------

void gles2_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetScissorRect* cmd = cb->allocCmd<SWCommand_SetScissorRect>();
    cmd->x = rect.x;
    cmd->y = rect.y;
    cmd->width = rect.width;
    cmd->height = rect.height;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetViewport* cmd = cb->allocCmd<SWCommand_SetViewport>();
    cmd->x = uint16(vp.x);
    cmd->y = uint16(vp.y);
    cmd->width = uint16(vp.width);
    cmd->height = uint16(vp.height);
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetFillMode* cmd = cb->allocCmd<SWCommand_SetFillMode>();
    cmd->mode = mode;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_FILLMODE, mode);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetVertexData* cmd = cb->allocCmd<SWCommand_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = uint16(streamIndex);
#else
    CommandBufferPool::Get(cmdBuf)->Command(CMD_SET_VERTEX_DATA, vb, streamIndex);
#endif
}

//------------------------------------------------------------------------------

static void gles2_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    if (buffer != InvalidHandle)
    {
#if RHI_GLES2__USE_CMDBUF_PACKING
        CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
        SWCommand_SetVertexProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetVertexProgConstBuffer>();
        cmd->buffer = buffer;
        cmd->bufIndex = bufIndex;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_VERTEX_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
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
        SWCommand_SetVertexTexture* cmd = cb->allocCmd<SWCommand_SetVertexTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_VERTEX_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetIndices* cmd = cb->allocCmd<SWCommand_SetIndices>();
    cmd->ib = ib;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_INDICES, ib);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetQueryIndex* cmd = cb->allocCmd<SWCommand_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_QUERY_INDEX, objectIndex);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetQueryBuffer* cmd = cb->allocCmd<SWCommand_SetQueryBuffer>();
    cmd->queryBuf = queryBuf;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_QUERY_BUFFER, queryBuf);
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
        SWCommand_SetFragmentProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetFragmentProgConstBuffer>();
        cmd->bufIndex = bufIndex;
        cmd->buffer = buffer;
        cmd->inst = ConstBufferGLES2::Instance(buffer);
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)));
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
        SWCommand_SetFragmentTexture* cmd = cb->allocCmd<SWCommand_SetFragmentTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
#else
        CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_FRAGMENT_TEXTURE, unitIndex, tex);
#endif
    }
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetDepthStencilState* cmd = cb->allocCmd<SWCommand_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_DEPTHSTENCIL_STATE, depthStencilState);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
// NOTE: expected to be called BEFORE SetFragmentTexture
#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_SetSamplerState* cmd = cb->allocCmd<SWCommand_SetSamplerState>();
    cmd->samplerState = samplerState;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_SET_SAMPLER_STATE, samplerState);
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
    SWCommand_DrawPrimitive* cmd = cb->allocCmd<SWCommand_DrawPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_DRAW_PRIMITIVE, uint32(mode), v_cnt);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    unsigned i_cnt = 0;
    int mode = _GLES2_GetDrawMode(type, count, &i_cnt);

#if RHI_GLES2__USE_CMDBUF_PACKING
    CommandBufferGLES2_t* cb = CommandBufferPoolGLES2::Get(cmdBuf);
    SWCommand_DrawIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawIndexedPrimitive>();
    cmd->mode = mode;
    cmd->indexCount = i_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_DRAW_INDEXED_PRIMITIVE, uint32(mode), v_cnt, firstVertex, startIndex);
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
    SWCommand_DrawInstancedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedPrimitive>();
    cmd->mode = mode;
    cmd->vertexCount = v_cnt;
    cmd->instanceCount = instCount;
    cmd->baseInstance = 0;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_DRAW_INSTANCED_PRIMITIVE, uint32(mode), instCount, v_cnt);
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
    SWCommand_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedIndexedPrimitive>();
    cmd->mode = mode;
    cmd->indexCount = v_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
    cmd->instanceCount = instCount;
    cmd->baseInstance = baseInstance;
#else
    CommandBufferPoolGLES2::Get(cmdBuf)->Command(CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE, uint32(mode), instCount, v_cnt, firstVertex, startIndex, baseInstance);
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

    int len = static_cast<int>(strlen(text));
    char* txt = reinterpret_cast<char*>(cb->text->Alloc(len / sizeof(float) + 2));

    memcpy(txt, text, len);
    txt[len] = '\n';
    txt[len + 1] = '\0';

#if RHI_GLES2__USE_CMDBUF_PACKING
    SWCommand_SetMarker* cmd = cb->allocCmd<SWCommand_SetMarker>();
    cmd->text = text;
#else
    cb->Command(CMD_SET_MARKER, (uint64)(txt));
#endif

#endif
}

//------------------------------------------------------------------------------

static Handle
gles2_SyncObject_Create()
{
    _GLES2_SyncObjectsSync.Lock();

    Handle handle = SyncObjectPoolGLES2::Alloc();
    SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(handle);

    sync->is_signaled = false;
    sync->is_used = false;

    _GLES2_SyncObjectsSync.Unlock();

    return handle;
}

//------------------------------------------------------------------------------

static void
gles2_SyncObject_Delete(Handle obj)
{
    _GLES2_SyncObjectsSync.Lock();

    SyncObjectPoolGLES2::Free(obj);

    _GLES2_SyncObjectsSync.Unlock();
}

//------------------------------------------------------------------------------

static bool
gles2_SyncObject_IsSignaled(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_GLES2_SyncObjectsSync);

    bool signaled = false;

    if (SyncObjectPoolGLES2::IsAlive(obj))
    {
        SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(obj);

        if (sync)
            signaled = sync->is_signaled;
    }
    else
    {
        signaled = true;
    }

    return signaled;
}

CommandBufferGLES2_t::CommandBufferGLES2_t()
    : isFirstInPass(true)
    , isLastInPass(true)
    , sync(InvalidHandle)
{
}

//------------------------------------------------------------------------------

CommandBufferGLES2_t::~CommandBufferGLES2_t()
{
}

//------------------------------------------------------------------------------



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
#else
    for (std::vector<uint64>::const_iterator c = _cmd.begin(), c_end = _cmd.end(); c != c_end; ++c)
#endif
    {
#if RHI_GLES2__USE_CMDBUF_PACKING
        const SWCommand* cmd = reinterpret_cast<const SWCommand*>(c);
        switch (cmd->type)
#else
        const uint64 cmd = *c;
        std::vector<uint64>::const_iterator arg = c + 1;

        if (cmd == EndCmd)
            break;
        switch (cmd)
#endif
        {
        case CMD_BEGIN:
        {
            GL_CALL(glFrontFace(GL_CW));
            GL_CALL(glEnable(GL_CULL_FACE));
            GL_CALL(glCullFace(GL_BACK));

            GL_CALL(glEnable(GL_DEPTH_TEST));
            GL_CALL(glDepthFunc(GL_LEQUAL));
            GL_CALL(glDepthMask(GL_TRUE));
            GL_CALL(glDisable(GL_SCISSOR_TEST));

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

                DVASSERT(cur_query_buf == InvalidHandle || !QueryBufferGLES2::QueryIsCompleted(cur_query_buf));
            }
        }
        break;

        case CMD_END:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            sync = (static_cast<const SWCommand_End*>(cmd))->syncObject;
            #else
            sync = Handle(arg[0]);
            c += 1;
            #endif

            if (isLastInPass)
            {
                if (cur_query_buf != InvalidHandle)
                    QueryBufferGLES2::QueryComplete(cur_query_buf);

#if defined(__DAVAENGINE_IPHONE__)
                if (_GLES2_Binded_FrameBuffer != _GLES2_Default_FrameBuffer) //defualt framebuffer is discard once after frame
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
        }
        break;

        case CMD_SET_VERTEX_DATA:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle vb = (static_cast<const SWCommand_SetVertexData*>(cmd))->vb;
            unsigned stream_i = (static_cast<const SWCommand_SetVertexData*>(cmd))->streamIndex;
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

        case CMD_SET_INDICES:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle ib = (static_cast<const SWCommand_SetIndices*>(cmd))->ib;
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

        case CMD_SET_QUERY_BUFFER:
        {
            DVASSERT(cur_query_buf == InvalidHandle);
            #if RHI_GLES2__USE_CMDBUF_PACKING
            cur_query_buf = (static_cast<const SWCommand_SetQueryBuffer*>(cmd))->queryBuf;
            #else
            cur_query_buf = (Handle)(arg[0]);
            c += 1;
            #endif
        }
        break;

        case CMD_SET_QUERY_INDEX:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            if (cur_query_buf != InvalidHandle)
                QueryBufferGLES2::SetQueryIndex(cur_query_buf, (static_cast<const SWCommand_SetQueryIndex*>(cmd))->objectIndex);
            #else
            if (cur_query_buf != InvalidHandle)
                QueryBufferGLES2::SetQueryIndex(cur_query_buf, uint32(arg[0]));
            c += 1;
            #endif
        }
        break;

        case CMD_SET_PIPELINE_STATE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle ps = (static_cast<const SWCommand_SetPipelineState*>(cmd))->ps;
            uint32 vdecl = (static_cast<const SWCommand_SetPipelineState*>(cmd))->vdecl;
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

        case CMD_SET_CULL_MODE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            CullMode mode = CullMode((static_cast<const SWCommand_SetCullMode*>(cmd))->mode);
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

        case CMD_SET_SCISSOR_RECT:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            GLint x = (static_cast<const SWCommand_SetScissorRect*>(cmd))->x;
            GLint y = (static_cast<const SWCommand_SetScissorRect*>(cmd))->y;
            GLsizei w = (static_cast<const SWCommand_SetScissorRect*>(cmd))->width;
            GLsizei h = (static_cast<const SWCommand_SetScissorRect*>(cmd))->height;
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

        case CMD_SET_VIEWPORT:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            GLint x = (static_cast<const SWCommand_SetViewport*>(cmd))->x;
            GLint y = (static_cast<const SWCommand_SetViewport*>(cmd))->y;
            GLsizei w = (static_cast<const SWCommand_SetViewport*>(cmd))->width;
            GLsizei h = (static_cast<const SWCommand_SetViewport*>(cmd))->height;
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

        case CMD_SET_FILLMODE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            FillMode mode = FillMode((static_cast<const SWCommand_SetFillMode*>(cmd))->mode);
            #else
            FillMode mode = FillMode(arg[0]);
            c += 1;
            #endif

            #if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, (mode == FILLMODE_WIREFRAME) ? GL_LINE : GL_FILL));
            #endif
        }
        break;

        case CMD_SET_DEPTHSTENCIL_STATE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle state = (static_cast<const SWCommand_SetDepthStencilState*>(cmd))->depthStencilState;
            #else
            Handle state = (Handle)(arg[0]);
            c += 1;
            #endif

            DepthStencilStateGLES2::SetToRHI(state);
        }
        break;

        case CMD_SET_SAMPLER_STATE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle state = (static_cast<const SWCommand_SetSamplerState*>(cmd))->samplerState;
            #else
            Handle state = (Handle)(arg[0]);
            c += 1;
            #endif

            SamplerStateGLES2::SetToRHI(state);
            StatSet::IncStat(stat_SET_SS, 1);
        }
        break;

        case CMD_SET_VERTEX_PROG_CONST_BUFFER:
        {
//++stcb_cnt;
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned buf_i = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->bufIndex;
            const void* inst = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->inst;
            Handle buf = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->buffer;
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

        case CMD_SET_FRAGMENT_PROG_CONST_BUFFER:
        {
//++stcb_cnt;
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned buf_i = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->bufIndex;
            const void* inst = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->inst;
            Handle buf = (static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd))->buffer;
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

        case CMD_SET_FRAGMENT_TEXTURE:
        {
//++sttx_cnt;
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle tex = (static_cast<const SWCommand_SetFragmentTexture*>(cmd))->tex;
            unsigned unit_i = (static_cast<const SWCommand_SetFragmentTexture*>(cmd))->unitIndex;
            #else
            Handle tex = (Handle)(arg[1]);
            unsigned unit_i = unsigned(arg[0]);
            c += 2;
            #endif

            TextureGLES2::SetToRHI(tex, unit_i, tex_unit_0);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case CMD_SET_VERTEX_TEXTURE:
        {
//++sttx_cnt;
            #if RHI_GLES2__USE_CMDBUF_PACKING
            Handle tex = (static_cast<const SWCommand_SetVertexTexture*>(cmd))->tex;
            unsigned unit_i = (static_cast<const SWCommand_SetVertexTexture*>(cmd))->unitIndex;
            #else
            Handle tex = (Handle)(arg[1]);
            unsigned unit_i = unsigned(arg[0]);
            c += 2;
            #endif

            TextureGLES2::SetToRHI(tex, unit_i, DAVA::InvalidIndex);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case CMD_DRAW_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DP")}
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = (static_cast<const SWCommand_DrawPrimitive*>(cmd))->vertexCount;
            int mode = (static_cast<const SWCommand_DrawPrimitive*>(cmd))->mode;
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
        }
        break;

        case CMD_DRAW_INDEXED_PRIMITIVE:
        {
//++dip_cnt;
//{SCOPED_NAMED_TIMING("gl.DIP")}
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = (static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd))->indexCount;
            int mode = (static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd))->mode;
            uint32 firstVertex = (static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd))->firstVertex;
            uint32 startIndex = (static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd))->startIndex;
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
        }
        break;

        case CMD_DRAW_INSTANCED_PRIMITIVE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = (static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd))->vertexCount;
            int mode = (static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd))->mode;
            unsigned instCount = (static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd))->instanceCount;
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

            if (vdecl_pending)
            {
                PipelineStateGLES2::SetVertexDeclToRHI(cur_ps, cur_vdecl, 0, countof(cur_vb), cur_vb);
                vdecl_pending = false;
            }

            #if defined(__DAVAENGINE_IPHONE__)
            GL_CALL(glDrawArraysInstancedEXT(mode, 0, v_cnt, instCount));
            #elif defined(__DAVAENGINE_ANDROID__)
            if (glDrawArraysInstanced)
            {
                GL_CALL(glDrawArraysInstanced(mode, 0, v_cnt, instCount));
            }
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
        }
        break;

        case CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            unsigned v_cnt = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->indexCount;
            int mode = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->mode;
            unsigned instCount = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->instanceCount;
            uint32 firstVertex = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->firstVertex;
            uint32 startIndex = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->startIndex;
            uint32 baseInst = (static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd))->baseInstance;
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
            if (glDrawElementsInstanced)
            {
                GL_CALL(glDrawElementsInstanced(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount));
            }
            #elif defined(__DAVAENGINE_MACOS__)
            GL_CALL(glDrawElementsInstancedBaseVertex(mode, v_cnt, i_sz, reinterpret_cast<void*>(uint64(i_off)), instCount, baseInst));
            #else
            GL_CALL(glDrawElementsInstancedBaseInstance(mode, v_cnt, i_sz, (void*)((uint64)i_off), instCount, baseInst));
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
        }
        break;

        case CMD_SET_MARKER:
        {
            #if RHI_GLES2__USE_CMDBUF_PACKING
            const char* text = (static_cast<const SWCommand_SetMarker*>(cmd))->text;
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
            RenderLoop::CheckImmediateCommand();
            immediate_cmd_ttw = 10;
        }
        
        #if RHI_GLES2__USE_CMDBUF_PACKING
        if (cmd->type == CMD_END)
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

static void _GLES2_RejectFrame(CommonImpl::Frame&& frame)
{
#ifdef __DAVAENGINE_ANDROID__

    if (frame.sync != InvalidHandle)
    {
        SyncObjectGLES2_t* s = SyncObjectPoolGLES2::Get(frame.sync);
        s->is_signaled = true;
        s->is_used = true;
    }
    for (std::vector<Handle>::iterator p = frame.pass.begin(), p_end = frame.pass.end(); p != p_end; ++p)
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
#endif
}

//------------------------------------------------------------------------------

static void _GLES2_ExecuteQueuedCommands(CommonImpl::Frame&& frame)
{
    StatSet::ResetAll();

    std::vector<RenderPassGLES2_t*> pass;
    unsigned frame_n = 0;

    for (std::vector<Handle>::iterator p = frame.pass.begin(), p_end = frame.pass.end(); p != p_end; ++p)
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

    frame_n = frame.frameNumber;

    if (frame.sync != InvalidHandle)
    {
        SyncObjectGLES2_t* sync = SyncObjectPoolGLES2::Get(frame.sync);
        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }

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
    for (std::vector<Handle>::iterator p = frame.pass.begin(), p_end = frame.pass.end(); p != p_end; ++p)
        RenderPassPoolGLES2::Free(*p);

    //update sync objects for gl
    _GLES2_SyncObjectsSync.Lock();
    for (SyncObjectPoolGLES2::Iterator s = SyncObjectPoolGLES2::Begin(), s_end = SyncObjectPoolGLES2::End(); s != s_end; ++s)
    {
        if (s->is_used && (frame_n - s->frame >= 2))
            s->is_signaled = true;
    }
    _GLES2_SyncObjectsSync.Unlock();
}

bool _GLES2_PresentBuffer()
{
    bool success = true;
    if (!_GLES2_Context) //this is special case when rendering is done inside other app render loop (eg: QT loop in ResEditor)
        return true;

// do swap-buffers            
    #if defined(__DAVAENGINE_WIN32__)
    SwapBuffers(_GLES2_WindowDC);    
    #elif defined(__DAVAENGINE_MACOS__)
    macos_gl_end_frame();
    #elif defined(__DAVAENGINE_IPHONE__)
    ios_gl_end_frame();
    #elif defined(__DAVAENGINE_ANDROID__)
    success = android_gl_end_frame();        
    #endif

    return success;
}

void _GLES2_ResetBlock()
{        
#if defined(__DAVAENGINE_ANDROID__)
    TextureGLES2::ReleaseAll();
    VertexBufferGLES2::ReleaseAll();
    IndexBufferGLES2::ReleaseAll();

    TextureGLES2::ReCreateAll();
    VertexBufferGLES2::ReCreateAll();
    IndexBufferGLES2::ReCreateAll();

    // update sync-objects, as pre-reset state is not actual anymore, also resolve constant reset causing already executed frame being never synced
    for (SyncObjectPoolGLES2::Iterator s = SyncObjectPoolGLES2::Begin(), s_end = SyncObjectPoolGLES2::End(); s != s_end; ++s)
    {
        if (s->is_used)
            s->is_signaled = true;
    }
#endif
}

void _GLES2_InvalidateFrameCache()
{
    ProgGLES2::InvalidateAllConstBufferInstances();
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

static void _GLES2_ExecImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    int err = GL_NO_ERROR;

#if defined(DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME)
    #define ACQUIRE_CONTEXT() _GLES2_AcquireContext()
    #define RELEASE_CONTEXT() _GLES2_ReleaseContext()
#else
    #define ACQUIRE_CONTEXT()
    #define RELEASE_CONTEXT()
#endif

    ACQUIRE_CONTEXT();

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
    GLCommand* commandData = reinterpret_cast<GLCommand*>(command->cmdData);
    for (GLCommand *cmd = commandData, *cmdEnd = commandData + command->cmdCount; cmd != cmdEnd; ++cmd)
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
            GL_CALL(glGenBuffers(GLsizei(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
            DVASSERT(*(reinterpret_cast<GLuint*>(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_BUFFER:
        {
            GL_CALL(glBindBuffer(GLenum(arg[0]), *(reinterpret_cast<GLuint*>(arg[1]))));
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
            GL_CALL(glDeleteBuffers(GLsizei(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_DATA:
        {
            GL_CALL(glBufferData(GLenum(arg[0]), GLsizei(arg[1]), reinterpret_cast<const void*>(arg[2]), GLenum(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::BUFFER_SUBDATA:
        {
            GL_CALL(glBufferSubData(GLenum(arg[0]), GLintptr(arg[1]), GLsizei(arg[2]), reinterpret_cast<const void*>(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_TEXTURES:
        {
            GL_CALL(glGenTextures(GLsizei(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
            DVASSERT(*(reinterpret_cast<GLuint*>(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_TEXTURES:
        {
            GL_CALL(glDeleteTextures(GLsizei(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
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
            GL_CALL(glBindTexture(GLenum(cmd->arg[0]), *(reinterpret_cast<GLuint*>(cmd->arg[1]))));
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
            GL_CALL(glTexParameteri(GLenum(arg[0]), GLenum(arg[1]), GLuint(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::TEX_IMAGE2D:
        {
            if (arg[10])
            {
                GL_CALL(glCompressedTexImage2D(GLenum(arg[0]), GLint(arg[1]), GLenum(arg[2]), GLsizei(arg[3]), GLsizei(arg[4]), GLint(arg[5]), GLsizei(arg[8]), reinterpret_cast<const GLvoid*>(arg[9])));
            }
            else
            {
                GL_CALL(glTexImage2D(GLenum(arg[0]), GLint(arg[1]), GLint(arg[2]), GLsizei(arg[3]), GLsizei(arg[4]), GLint(arg[5]), GLenum(arg[6]), GLenum(arg[7]), reinterpret_cast<const GLvoid*>(arg[9])));
            }
            cmd->status = err;
        }
        break;

        case GLCommand::GENERATE_MIPMAP:
        {
            GL_CALL(glGenerateMipmap(GLenum(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::PIXEL_STORE_I:
        {
            GL_CALL(glPixelStorei(GLenum(arg[0]), GLint(arg[1])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::READ_PIXELS:
        {
            GL_CALL(glReadPixels(GLint(arg[0]), GLint(arg[1]), GLsizei(arg[2]), GLsizei(arg[3]), GLenum(arg[4]), GLenum(arg[5]), reinterpret_cast<GLvoid*>(arg[6])));
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
            GL_CALL(cmd->retval = glCreateShader(GLenum(arg[0])));
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
            GL_CALL(glShaderSource(GLuint(arg[0]), GLsizei(arg[1]), reinterpret_cast<const GLchar**>(arg[2]), reinterpret_cast<const GLint*>(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::COMPILE_SHADER:
        {
            GL_CALL(glCompileShader(GLuint(arg[0])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_IV:
        {
            GL_CALL(glGetShaderiv(GLuint(arg[0]), GLenum(arg[1]), reinterpret_cast<GLint*>(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_SHADER_INFO_LOG:
        {
            GL_CALL(glGetShaderInfoLog(GLuint(arg[0]), GLsizei(arg[1]), reinterpret_cast<GLsizei*>(arg[2]), reinterpret_cast<GLchar*>(arg[3])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_PROGRAM_IV:
        {
            GL_CALL(glGetProgramiv(GLuint(arg[0]), GLenum(arg[1]), reinterpret_cast<GLint*>(arg[2])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_ATTRIB_LOCATION:
        {
            GL_CALL(cmd->retval = glGetAttribLocation(GLuint(arg[0]), reinterpret_cast<const GLchar*>(arg[1])));
            cmd->status = 0;
        }
        break;

        case GLCommand::GET_ACTIVE_UNIFORM:
        {
            GL_CALL(glGetActiveUniform(GLuint(arg[0]), GLuint(arg[1]), GLsizei(arg[2]), reinterpret_cast<GLsizei*>(arg[3]), reinterpret_cast<GLint*>(arg[4]), reinterpret_cast<GLenum*>(arg[5]), reinterpret_cast<GLchar*>(arg[6])));
            cmd->status = err;
        }
        break;

        case GLCommand::GET_UNIFORM_LOCATION:
        {
            GL_CALL(cmd->retval = glGetUniformLocation(GLuint(arg[0]), reinterpret_cast<const GLchar*>(arg[1])));
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
            GL_CALL(glGenFramebuffers(GLuint(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::GEN_RENDERBUFFERS:
        {
            GL_CALL(glGenRenderbuffers(GLuint(arg[0]), reinterpret_cast<GLuint*>(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_RENDERBUFFERS:
        {
            GL_CALL(glDeleteRenderbuffers(GLsizei(arg[0]), reinterpret_cast<const GLuint*>(arg[1])));
        }
        break;

        case GLCommand::BIND_FRAMEBUFFER:
        {
            GL_CALL(glBindFramebuffer(GLenum(arg[0]), GLuint(arg[1])));
            cmd->status = err;
        }
        break;

        case GLCommand::BIND_RENDERBUFFER:
        {
            GL_CALL(glBindRenderbuffer(GLenum(arg[0]), GLuint(arg[1])));
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
            GL_CALL(glDeleteFramebuffers(GLsizei(arg[0]), reinterpret_cast<const GLuint*>(arg[1])));
            cmd->retval = 0;
            cmd->status = err;
        }
        break;

        case GLCommand::DRAWBUFFERS:
        {
                #if defined __DAVAENGINE_IPHONE__ || defined __DAVAENGINE_ANDROID__
                #else
            GL_CALL(glDrawBuffers(GLuint(arg[0]), reinterpret_cast<GLenum*>(arg[1])));
            cmd->status = err;
                #endif
        }
        break;

        case GLCommand::GET_QUERYOBJECT_UIV:
        {
#if defined(__DAVAENGINE_IPHONE__)
            EXEC_GL(glGetQueryObjectuivEXT(GLuint(arg[0]), GLenum(arg[1]), (GLuint*)(arg[2])));
#elif defined(__DAVAENGINE_ANDROID__)
#else
            EXEC_GL(glGetQueryObjectuiv(GLuint(arg[0]), GLenum(arg[1]), (GLuint*)(arg[2])));
#endif
            cmd->status = err;
        }
        break;

        case GLCommand::DELETE_QUERIES:
        {
#if defined(__DAVAENGINE_IPHONE__)
            EXEC_GL(glDeleteQueriesEXT(GLsizei(arg[0]), (const GLuint*)(arg[1])));
#elif defined(__DAVAENGINE_ANDROID__)
#else
            EXEC_GL(glDeleteQueries(GLsizei(arg[0]), (const GLuint*)(arg[1])));
#endif
            cmd->status = err;
        }
        break;

        case GLCommand::GET_QUERY_RESULT_NO_WAIT:
        {
            GLuint result = 0;

#if defined(__DAVAENGINE_IPHONE__)
            EXEC_GL(glGetQueryObjectuivEXT(GLuint(arg[0]), GL_QUERY_RESULT_AVAILABLE, &result));
#elif defined(__DAVAENGINE_ANDROID__)
#else
            EXEC_GL(glGetQueryObjectuiv(GLuint(arg[0]), GL_QUERY_RESULT_AVAILABLE, &result));
#endif
            cmd->status = err;

            if (err == GL_NO_ERROR && result)
            {
#if defined(__DAVAENGINE_IPHONE__)
                EXEC_GL(glGetQueryObjectuivEXT(GLuint(arg[0]), GL_QUERY_RESULT, (GLuint*)(arg[1])));
#elif defined(__DAVAENGINE_ANDROID__)
#else
                EXEC_GL(glGetQueryObjectuiv(GLuint(arg[0]), GL_QUERY_RESULT, (GLuint*)(arg[1])));
#endif
            }
        }
        break;
        }
    }
    RELEASE_CONTEXT();
#undef EXEC_GL
}

//------------------------------------------------------------------------------

void ExecGL(GLCommand* command, uint32 cmdCount, bool forceImmediate)
{
    CommonImpl::ImmediateCommand cmd;
    cmd.cmdData = command;
    cmd.cmdCount = cmdCount;
    cmd.forceImmediate = forceImmediate;
    RenderLoop::IssueImmediateCommand(&cmd);
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

    DispatchPlatform::ProcessImmediateCommand = _GLES2_ExecImmediateCommand;
    DispatchPlatform::ExecuteFrame = _GLES2_ExecuteQueuedCommands;
    DispatchPlatform::RejectFrame = _GLES2_RejectFrame;
    DispatchPlatform::PresntBuffer = _GLES2_PresentBuffer;
    DispatchPlatform::ResetBlock = _GLES2_ResetBlock;
    DispatchPlatform::FinishFrame = _GLES2_InvalidateFrameCache;
}
}

} // namespace rhi
