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
    CMD_ISSUE_TIMESTAMP_QUERY = 17,

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
    CMD_DRAW_INDEXED_PRIMITIVE_RANGED = 43,
    CMD_DRAW_INSTANCED_PRIMITIVE = 46,
    CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE = 47,
    CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE_RANGED = 48,

    CMD_SET_MARKER = 51,
};

#if defined(__DAVAENGINE_WIN32__)
#pragma pack(push, 1)
#endif

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
#define DV_ATTR_PACKED __attribute__((packed))
#else
#define DV_ATTR_PACKED 
#endif

struct SWCommand
{
    uint8 type;
    uint8 size;

    SWCommand(uint8 t, uint8 sz)
        : type(t)
        , size(sz)
    {
    }
} DV_ATTR_PACKED;

template <class T, SoftwareCommandType t>
struct SWCommandImpl : public SWCommand
{
    SWCommandImpl()
        : SWCommand(t, sizeof(T))
    {
    }
} DV_ATTR_PACKED;

struct SWCommand_Begin : public SWCommandImpl<SWCommand_Begin, CMD_BEGIN>
{
} DV_ATTR_PACKED;

struct SWCommand_End : public SWCommandImpl<SWCommand_End, CMD_END>
{
    Handle syncObject;
    bool doCommit;
} DV_ATTR_PACKED;

struct SWCommand_SetVertexData : public SWCommandImpl<SWCommand_SetVertexData, CMD_SET_VERTEX_DATA>
{
    uint16 streamIndex;
    Handle vb;
} /*DV_ATTR_PACKED*/;

struct SWCommand_SetIndices : public SWCommandImpl<SWCommand_SetIndices, CMD_SET_INDICES>
{
    Handle ib;
} DV_ATTR_PACKED;

struct SWCommand_SetQueryBuffer : public SWCommandImpl<SWCommand_SetQueryBuffer, CMD_SET_QUERY_BUFFER>
{
    Handle queryBuf;
} DV_ATTR_PACKED;

struct SWCommand_SetQueryIndex : public SWCommandImpl<SWCommand_SetQueryIndex, CMD_SET_QUERY_INDEX>
{
    uint32 objectIndex;
} DV_ATTR_PACKED;

struct SWCommand_IssueTimestamptQuery : public SWCommandImpl<SWCommand_IssueTimestamptQuery, CMD_ISSUE_TIMESTAMP_QUERY>
{
    Handle querySet;
    uint32 timestampIndex;
} DV_ATTR_PACKED;

struct SWCommand_SetPipelineState : public SWCommandImpl<SWCommand_SetPipelineState, CMD_SET_PIPELINE_STATE>
{
    uint32 vdecl;
    uint32 ps;
} DV_ATTR_PACKED;

struct SWCommand_SetDepthStencilState : public SWCommandImpl<SWCommand_SetDepthStencilState, CMD_SET_DEPTHSTENCIL_STATE>
{
    Handle depthStencilState;
} DV_ATTR_PACKED;

struct SWCommand_SetSamplerState : public SWCommandImpl<SWCommand_SetSamplerState, CMD_SET_SAMPLER_STATE>
{
    Handle samplerState;
} DV_ATTR_PACKED;

struct SWCommand_SetCullMode : public SWCommandImpl<SWCommand_SetCullMode, CMD_SET_CULL_MODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct SWCommand_SetScissorRect : public SWCommandImpl<SWCommand_SetScissorRect, CMD_SET_SCISSOR_RECT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct SWCommand_SetViewport : public SWCommandImpl<SWCommand_SetViewport, CMD_SET_VIEWPORT>
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
} /*DV_ATTR_PACKED*/;

struct SWCommand_SetFillMode : public SWCommandImpl<SWCommand_SetFillMode, CMD_SET_FILLMODE>
{
    uint8 mode;
} DV_ATTR_PACKED;

struct SWCommand_SetVertexProgConstBuffer : public SWCommandImpl<SWCommand_SetVertexProgConstBuffer, CMD_SET_VERTEX_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct SWCommand_SetFragmentProgConstBuffer : public SWCommandImpl<SWCommand_SetFragmentProgConstBuffer, CMD_SET_FRAGMENT_PROG_CONST_BUFFER>
{
    uint8 bufIndex;
    Handle buffer;
    const void* inst;
} DV_ATTR_PACKED;

struct SWCommand_SetVertexTexture : public SWCommandImpl<SWCommand_SetVertexTexture, CMD_SET_VERTEX_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct SWCommand_SetFragmentTexture : public SWCommandImpl<SWCommand_SetFragmentTexture, CMD_SET_FRAGMENT_TEXTURE>
{
    uint8 unitIndex;
    Handle tex;
} DV_ATTR_PACKED;

struct SWCommand_DrawPrimitive : public SWCommandImpl<SWCommand_DrawPrimitive, CMD_DRAW_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
} DV_ATTR_PACKED;

struct SWCommand_DrawInstancedPrimitive : public SWCommandImpl<SWCommand_DrawInstancedPrimitive, CMD_DRAW_INSTANCED_PRIMITIVE>
{
    uint8 mode;
    uint32 vertexCount;
    uint16 instanceCount;
    uint16 baseInstance;
} DV_ATTR_PACKED;

struct SWCommand_DrawIndexedPrimitive : public SWCommandImpl<SWCommand_DrawIndexedPrimitive, CMD_DRAW_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 indexCount;
    uint32 firstVertex;
    uint32 startIndex;
} DV_ATTR_PACKED;

struct SWCommand_DrawIndexedPrimitiveRanged : public SWCommandImpl<SWCommand_DrawIndexedPrimitiveRanged, CMD_DRAW_INDEXED_PRIMITIVE_RANGED>
{
    uint8 mode;
    uint32 indexCount;
    uint32 firstVertex;
    uint32 startIndex;
    uint32 vertexCount;
} DV_ATTR_PACKED;

struct SWCommand_DrawInstancedIndexedPrimitive : public SWCommandImpl<SWCommand_DrawInstancedIndexedPrimitive, CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE>
{
    uint8 mode;
    uint32 indexCount;
    uint32 firstVertex;
    uint32 startIndex;
    uint16 instanceCount;
    uint16 baseInstance;
} DV_ATTR_PACKED;

struct SWCommand_DrawInstancedIndexedPrimitiveRanged : public SWCommandImpl<SWCommand_DrawInstancedIndexedPrimitiveRanged, CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE_RANGED>
{
    uint8 mode;
    uint32 indexCount;
    uint32 firstVertex;
    uint32 startIndex;
    uint32 vertexCount;
    uint16 instanceCount;
    uint16 baseInstance;
} DV_ATTR_PACKED;

struct SWCommand_SetMarker : public SWCommandImpl<SWCommand_SetMarker, CMD_SET_MARKER>
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
    RingBuffer* text = nullptr;
};

inline void SoftwareCommandBuffer::Begin()
{
    curUsedSize = 0;
}

inline void SoftwareCommandBuffer::End()
{
}
}