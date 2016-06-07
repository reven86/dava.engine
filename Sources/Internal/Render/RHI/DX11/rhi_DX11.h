#ifndef __RHI_DX11_H__
#define __RHI_DX11_H__

#include "../rhi_Public.h"
#include "../Common/rhi_Private.h"
#include "../Common/rhi_Impl.h"
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
void SetToRHI(Handle vb, unsigned stream_i, unsigned offset, unsigned stride, ID3D11DeviceContext* context);
}

namespace IndexBufferDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void SetToRHI(Handle vb, unsigned offset, ID3D11DeviceContext* context);
}

namespace QueryBufferDX11
{
void SetupDispatch(Dispatch* dispatch);

void BeginQuery(Handle buf, uint32 objectIndex, ID3D11DeviceContext* context);
void EndQuery(Handle buf, uint32 objectIndex, ID3D11DeviceContext* context);
}

namespace PerfQuerySetDX11
{
void SetupDispatch(Dispatch* dispatch);
void BeginFreqMeasurment(Handle handle, ID3D11DeviceContext* context);
void EndFreqMeasurment(Handle handle, ID3D11DeviceContext* context);
void IssueTimestampQuery(Handle handle, uint32 timestampIndex, ID3D11DeviceContext* context);
void IssueFrameBeginQuery(Handle handle, ID3D11DeviceContext* context);
void IssueFrameEndQuery(Handle handle, ID3D11DeviceContext* context);
Handle Current();
void ObtainResults(Handle handle);
}

namespace PipelineStateDX11
{
void SetupDispatch(Dispatch* dispatch);
unsigned VertexLayoutStride(Handle ps, unsigned stream_i);
unsigned VertexLayoutStreamCount(Handle ps);
void GetConstBufferCount(Handle ps, unsigned* vertexBufCount, unsigned* fragmentBufCount);
void SetToRHI(Handle ps, uint32 layoutUID, ID3D11DeviceContext* context);
}

namespace ConstBufferDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void InitializeRingBuffer(uint32 size);
void InvalidateAll();
#if RHI_DX11__USE_DEFERRED_CONTEXTS
void SetToRHI(Handle cb, ID3D11DeviceContext* context, ID3D11Buffer** buffer);
#else
void SetToRHI(Handle cb, const void* instData);
const void* Instance(Handle cb);
void InvalidateAllInstances();
#endif
}

namespace TextureDX11
{
void Init(uint32 maxCount);
void SetupDispatch(Dispatch* dispatch);
void SetToRHIFragment(Handle tex, unsigned unitIndex, ID3D11DeviceContext* context);
void SetToRHIVertex(Handle tex, unsigned unitIndex, ID3D11DeviceContext* context);
void SetRenderTarget(Handle color, Handle depthstencil, unsigned level, TextureFace face, ID3D11DeviceContext* context);
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
void DiscardAll();
}

struct
DX11Command
{
    enum Func
    {
        NOP = 0,

        MAP = 1,
        UNMAP = 2,
        UPDATE_SUBRESOURCE = 3,
        COPY_RESOURCE = 4
    };

    Func func;
    uint64 arg[12];
    long retval;
};

void ExecDX11(DX11Command* cmd, uint32 cmdCount, bool force_immediate = false);

//==============================================================================
}
#endif // __RHI_DX11_H__
