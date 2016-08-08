#pragma once
#include "../rhi_Type.h"
#include "rhi_RingBuffer.h"

namespace rhi
{
enum SoftwareCommandType
{
    CMD_BEGIN = 1,
    CMD_END = 2,

    CMD_SET_VERTEX_DATA = 11,
    CMD_SET_INDICES = 12,
    CMD_SET_QUERY_BUFFER = 13,
    CMD_SET_QUERY_INDEX = 14,

    CMD_SET_PIPELINE_STATE = 21,
    CMD_SET_DEPTHSTENCIL_STATE = 22,
    CMD_SET_SAMPLER_STATE = 23,
    CMD_SET_CULL_MODE = 24,
    CMD_SET_SCISSOR_RECT = 25,
    CMD_SET_VIEWPORT = 26,
    CMD_SET_FILLMODE = 27,

    CMD_SET_VERTEX_PROG_CONST_BUFFER = 31,
    CMD_SET_FRAGMENT_PROG_CONST_BUFFER = 32,
    CMD_SET_VERTEX_TEXTURE = 33,
    CMD_SET_FRAGMENT_TEXTURE = 34,

    CMD_DRAW_PRIMITIVE = 41,
    CMD_DRAW_INDEXED_PRIMITIVE = 42,
    CMD_DRAW_INSTANCED_PRIMITIVE = 43,
    CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE = 44,

    CMD_SET_MARKER = 51,

    CMD_NOP = 77
};

#if defined(__DAVAENGINE_WIN32__)
#pragma pack(push, 1)
#endif

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#define DV_ATTR_PACKED __attribute__((packed))
#else
#define DV_ATTR_PACKED 
#endif

struct CommandGLES2
{
    uint8 type;
    uint8 size;

    CommandGLES2(uint8 t, uint8 sz)
        : type(t)
        , size(sz)
    {
    }
} DV_ATTR_PACKED;

template <class T, SoftwareCommandType t>
struct CommandGLES2Impl : public CommandGLES2
{
    CommandGLES2Impl()
        : CommandGLES2(t, sizeof(T))
    {
    }
} DV_ATTR_PACKED;

struct CommandGLES2_Begin : public CommandGLES2Impl<CommandGLES2_Begin, CMD_BEGIN>
{
} DV_ATTR_PACKED;

struct CommandGLES2_End : public CommandGLES2Impl<CommandGLES2_End, CMD_END>
{
    Handle syncObject;
} DV_ATTR_PACKED;

struct CommandGLES2_SetVertexData : public CommandGLES2Impl<CommandGLES2_SetVertexData, CMD_SET_VERTEX_DATA>
{
    uint16 streamIndex;
    Handle vb;
} /*DV_ATTR_PACKED*/;

struct CommandGLES2_SetIndices : public CommandGLES2Impl<CommandGLES2_SetIndices, CMD_SET_INDICES>
{
    Handle ib;
} DV_ATTR_PACKED;

struct CommandGLES2_SetQueryBuffer : public CommandGLES2Impl<CommandGLES2_SetQueryBuffer, CMD_SET_QUERY_BUFFER>
{
    Handle queryBuf;
} DV_ATTR_PACKED;

struct CommandGLES2_SetQueryIndex : public CommandGLES2Impl<CommandGLES2_SetQueryIndex, CMD_SET_QUERY_INDEX>
{
    uint32 objectIndex;
} DV_ATTR_PACKED;

struct CommandGLES2_SetPipelineState : public CommandGLES2Impl<CommandGLES2_SetPipelineState, CMD_SET_PIPELINE_STATE>
{
    uint32 vdecl;
    uint32 ps;
} DV_ATTR_PACKED;

struct CommandGLES2_SetDepthStencilState : public CommandGLES2Impl<CommandGLES2_SetDepthStencilState, CMD_SET_DEPTHSTENCIL_STATE>
{
    Handle depthStencilState;
} DV_ATTR_PACKED;

struct CommandGLES2_SetSamplerState : public CommandGLES2Impl<CommandGLES2_SetSamplerState, CMD_SET_SAMPLER_STATE>
{
    Handle samplerState;
} DV_ATTR_PACKED;

struct CommandGLES2_SetCullMode : public CommandGLES2Impl<CommandGLES2_SetCullMode, CMD_SET_CULL_MODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct CommandGLES2_SetScissorRect : public CommandGLES2Impl<CommandGLES2_SetScissorRect, CMD_SET_SCISSOR_RECT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct CommandGLES2_SetViewport : public CommandGLES2Impl<CommandGLES2_SetViewport, CMD_SET_VIEWPORT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct CommandGLES2_SetFillMode : public CommandGLES2Impl<CommandGLES2_SetFillMode, CMD_SET_FILLMODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct CommandGLES2_SetVertexProgConstBuffer : public CommandGLES2Impl<CommandGLES2_SetVertexProgConstBuffer, CMD_SET_VERTEX_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct CommandGLES2_SetFragmentProgConstBuffer : public CommandGLES2Impl<CommandGLES2_SetFragmentProgConstBuffer, CMD_SET_FRAGMENT_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct CommandGLES2_SetVertexTexture : public CommandGLES2Impl<CommandGLES2_SetVertexTexture, CMD_SET_VERTEX_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct CommandGLES2_SetFragmentTexture : public CommandGLES2Impl<CommandGLES2_SetFragmentTexture, CMD_SET_FRAGMENT_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct CommandGLES2_DrawPrimitive : public CommandGLES2Impl<CommandGLES2_DrawPrimitive, CMD_DRAW_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
} DV_ATTR_PACKED;

struct CommandGLES2_DrawInstancedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawInstancedPrimitive, CMD_DRAW_INSTANCED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint16 instanceCount;
    uint16 baseInstance;
} DV_ATTR_PACKED;

struct CommandGLES2_DrawIndexedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawIndexedPrimitive, CMD_DRAW_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint32 firstVertex;
    uint32 startIndex;
} DV_ATTR_PACKED;

struct CommandGLES2_DrawInstancedIndexedPrimitive : public CommandGLES2Impl<CommandGLES2_DrawInstancedIndexedPrimitive, CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint32 firstVertex;
    uint32 startIndex;
    uint16 instanceCount;
    uint16 baseInstance;
} DV_ATTR_PACKED;

struct CommandGLES2_SetMarker : public CommandGLES2Impl<CommandGLES2_SetMarker, CMD_SET_MARKER>
{
    const char* text;
};

#ifdef __DAVAENGINE_WIN32__
#pragma pack(pop)
#endif

struct SoftwareCommandBuffer
{
public:
    void Begin();
    void End();

    RingBuffer* text = nullptr;

    template <class T>
    inline T* allocCmd()
    {
        if (curUsedSize + sizeof(T) >= cmdDataSize)
        {
            cmdDataSize += 4 * 1024; // CRAP: hardcoded grow-size
            cmdData = reinterpret_cast<uint8*>(::realloc(cmdData, cmdDataSize));
        }

        uint8* p = cmdData + curUsedSize;
        curUsedSize += sizeof(T);

        return new (reinterpret_cast<T*>(p)) T();
    }

    uint8* cmdData = nullptr;
    uint32 cmdDataSize = 0;
    uint32 curUsedSize = 0;
};

struct SoftwareCommandBufferUnpacked
{
public:
    void Begin();
    void End();

    void Command(uint64 cmd);
    void Command(uint64 cmd, uint64 arg1);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5);
    void Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6);

    std::vector<uint64> _cmd;
    static const uint64 EndCmd = 0xFFFFFFFF;
    RingBuffer* text = nullptr;
};
}