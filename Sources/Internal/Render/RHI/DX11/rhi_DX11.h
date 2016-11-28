#pragma once

#include "../rhi_Public.h"
#include "../Common/rhi_Private.h"
#include "../Common/rhi_BackendImpl.h"
#include "_dx11.h"

struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace rhi
{
void dx11_Initialize(const InitParam& param);

namespace VertexBufferDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void SetToRHI(Handle vb, uint32 stream_i, uint32 offset, uint32 stride, ID3D11DeviceContext* context);
}

namespace IndexBufferDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void SetToRHI(Handle vb, uint32 offset, ID3D11DeviceContext* context);
}

namespace QueryBufferDX11
{
void SetupDispatch(Dispatch* dispatch);
void SetQueryIndex(Handle buf, uint32 objectIndex, ID3D11DeviceContext* context);
void QueryComplete(Handle buf, ID3D11DeviceContext* context);
bool QueryIsCompleted(Handle buf);

void ReleaseQueryPool();
}

namespace PerfQueryDX11
{
void IssueTimestampQuery(Handle handle, ID3D11DeviceContext* context);
void BeginMeasurment(ID3D11DeviceContext* context);
void EndMeasurment(ID3D11DeviceContext* context);
void DeferredPerfQueriesIssued(const std::vector<Handle>& queries);
void IssueTimestampQueryDeferred(Handle handle, ID3D11DeviceContext* context);
void SetupDispatch(Dispatch* dispatch);
void ObtainPerfQueryMeasurment(ID3D11DeviceContext* context);
void ReleasePerfQueryPool();
}

namespace PipelineStateDX11
{
void SetupDispatch(Dispatch* dispatch);
uint32 VertexLayoutStride(Handle ps, uint32 stream_i);
uint32 VertexLayoutStreamCount(Handle ps);
void GetConstBufferCount(Handle ps, uint32* vertexBufCount, uint32* fragmentBufCount);
void SetToRHI(Handle ps, uint32 layoutUID, ID3D11DeviceContext* context);
}

namespace ConstBufferDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void InitializeRingBuffer(uint32 size);
void InvalidateAll();
void SetToRHI(Handle cb, ID3D11DeviceContext* context, ID3D11Buffer** buffer);
void SetToRHI(Handle cb, const void* instData);
const void* Instance(Handle cb);
void InvalidateAllInstances();
}

namespace TextureDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void SetToRHIFragment(Handle tex, uint32 unitIndex, ID3D11DeviceContext* context);
void SetToRHIVertex(Handle tex, uint32 unitIndex, ID3D11DeviceContext* context);
void SetRenderTarget(Handle color, Handle depthstencil, uint32 level, TextureFace face, ID3D11DeviceContext* context);
void ResolveMultisampling(Handle from, Handle to, ID3D11DeviceContext* context);
Size2i Size(Handle tex);
}

namespace DepthStencilStateDX11
{
void SetupDispatch(Dispatch* dispatch);
void SetToRHI(Handle state, ID3D11DeviceContext* context);
}

namespace SamplerStateDX11
{
void SetupDispatch(Dispatch* dispatch);
void SetToRHI(Handle state, ID3D11DeviceContext* context);
}

namespace RenderPassDX11
{
void SetupDispatch(Dispatch* dispatch);
}

namespace CommandBufferDX11
{
void SetupDispatch(Dispatch* dispatch);
}

struct DX11Command
{
    enum Func : uint32_t
    {
        NOP,

        MAP,
        UNMAP,
        UPDATE_SUBRESOURCE,
        COPY_RESOURCE,
        SYNC_CPU_GPU,

        /*
         * Device commands (invokes _D3D11_Device method)
         */
        QUERY_INTERFACE = 0x1000,
        CREATE_DEFERRED_CONTEXT,

        CREATE_BLEND_STATE,
        CREATE_SAMPLER_STATE,
        CREATE_RASTERIZER_STATE,
        CREATE_DEPTH_STENCIL_STATE,

        CREATE_VERTEX_SHADER,
        CREATE_PIXEL_SHADER,
        CREATE_INPUT_LAYOUT,

        CREATE_QUERY,
        CREATE_BUFFER,

        CREATE_TEXTURE_2D,
        CREATE_RENDER_TARGET_VIEW,
        CREATE_DEPTH_STENCIL_VIEW,
        CREATE_SHADER_RESOURCE_VIEW,

        // service values for range checking
        DEVICE_LAST_COMMAND,
        DEVICE_FIRST_COMMAND = QUERY_INTERFACE
    };

    struct Arguments
    {
        DAVA::uint64 arg[12];
    };

    Func func = Func::NOP;
    HRESULT retval = S_OK;
    Arguments arguments;

    template <class... args>
    DX11Command(Func f, args&&... a)
        : func(f)
        , arguments({ DAVA::uint64(a)... })
    {
    }
};

void ValidateDX11Device(const char* call);
void ExecDX11(DX11Command* cmd, uint32 cmdCount, bool force_immediate = false);

bool ExecDX11DeviceCommand(DX11Command cmd, const char* cmdName, const char* fileName, DAVA::uint32 line);

#define DX11DeviceCommand(CMD, ...) ExecDX11DeviceCommand(DX11Command(CMD, __VA_ARGS__), #CMD, __FILE__, __LINE__)
#define DX11Check(HR) DX11_CheckResult(HR, #HR, __FILE__, __LINE__)
}
