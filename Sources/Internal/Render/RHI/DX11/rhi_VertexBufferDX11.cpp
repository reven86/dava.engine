#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_dx11.h"

namespace rhi
{
//==============================================================================

class
VertexBufferDX11_t
{
public:
    VertexBufferDX11_t();
    ~VertexBufferDX11_t();

    unsigned size;
    Usage usage;
    ID3D11Buffer* buffer;
#if !RHI_DX11__USE_DEFERRED_CONTEXTS
    void* mappedData;
    uint32 updatePending : 1;
#endif
    uint32 isMapped : 1;
};

typedef ResourcePool<VertexBufferDX11_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, true> VertexBufferDX11Pool;

RHI_IMPL_POOL(VertexBufferDX11_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, true);

VertexBufferDX11_t::VertexBufferDX11_t()
    : size(0)
    , usage(USAGE_STATICDRAW)
    , buffer(nullptr)
#if !RHI_DX11__USE_DEFERRED_CONTEXTS
    , mappedData(nullptr)
    , updatePending(false)
#endif
    , isMapped(false)
{
}

//------------------------------------------------------------------------------

VertexBufferDX11_t::~VertexBufferDX11_t()
{
}

//==============================================================================

//------------------------------------------------------------------------------

static Handle
dx11_VertexBuffer_Create(const VertexBuffer::Descriptor& desc)
{
    Handle handle = InvalidHandle;

    DVASSERT(desc.size);
    if (desc.size)
    {
        D3D11_BUFFER_DESC desc11 = { 0 };
        ID3D11Buffer* buf = nullptr;
        D3D11_SUBRESOURCE_DATA data;

        desc11.ByteWidth = desc.size;
        desc11.Usage = D3D11_USAGE_DYNAMIC;
        desc11.CPUAccessFlags = 0;
        desc11.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc11.MiscFlags = 0;

        switch (desc.usage)
        {
        case USAGE_DEFAULT:
            desc11.Usage = D3D11_USAGE_DYNAMIC;
            desc11.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;

        case USAGE_STATICDRAW:
            desc11.Usage = D3D11_USAGE_IMMUTABLE;
            DVASSERT(desc.initialData);
            break;

        case USAGE_DYNAMICDRAW:
            desc11.Usage = D3D11_USAGE_DYNAMIC;
            desc11.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;
        }

        if (desc.initialData)
        {
            data.pSysMem = desc.initialData;
            data.SysMemPitch = desc.size;
        }

        HRESULT hr = _D3D11_Device->CreateBuffer(&desc11, (desc.initialData) ? &data : NULL, &buf);
        CHECK_HR(hr)

        if (SUCCEEDED(hr))
        {
            handle = VertexBufferDX11Pool::Alloc();
            VertexBufferDX11_t* vb = VertexBufferDX11Pool::Get(handle);

            vb->size = desc.size;
            vb->usage = desc.usage;
            vb->buffer = buf;
            vb->isMapped = false;
        }
        else
        {
            Logger::Error("FAILED to create vertex-buffer:\n%s\n", D3D11ErrorText(hr));
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
dx11_VertexBuffer_Delete(Handle vb)
{
    if (vb != InvalidHandle)
    {
        VertexBufferDX11_t* self = VertexBufferDX11Pool::Get(vb);

        if (self->buffer)
        {
            self->buffer->Release();
            self->buffer = nullptr;
        }

        #if !RHI_DX11__USE_DEFERRED_CONTEXTS
        if (self->mappedData)
        {
            ::free(self->mappedData);
            self->mappedData = nullptr;
        }
        #endif

        self->size = 0;

        VertexBufferDX11Pool::Free(vb);
    }
}

//------------------------------------------------------------------------------

static bool
dx11_VertexBuffer_Update(Handle vb, const void* data, unsigned offset, unsigned size)
{
    bool success = false;
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get(vb);

    DVASSERT(self->usage != USAGE_DYNAMICDRAW);
    DVASSERT(self->usage != USAGE_STATICDRAW);
    DVASSERT(!self->isMapped);

    if (self->usage == USAGE_DEFAULT)
    {
        if (offset + size <= self->size)
        {
            D3D11_MAPPED_SUBRESOURCE rc = { 0 };
            DX11Command cmd1 = { DX11Command::MAP, { uint64(self->buffer), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64(&rc) } };

            ExecDX11(&cmd1, 1);
            CHECK_HR(cmd1.retval)

            if (rc.pData)
            {
                DX11Command cmd2 = { DX11Command::UNMAP, { uint64(self->buffer), 0 } };

                memcpy(((uint8*)(rc.pData)) + offset, data, size);
                ExecDX11(&cmd2, 1);
                success = true;
            }
        }
    }

    return success;
}

//------------------------------------------------------------------------------

static void*
dx11_VertexBuffer_Map(Handle vb, unsigned offset, unsigned size)
{
    void* ptr = nullptr;
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get(vb);

    DVASSERT(self->usage != USAGE_STATICDRAW);
    DVASSERT(!self->isMapped);

    if (self->usage == USAGE_DYNAMICDRAW)
    {
        D3D11_MAPPED_SUBRESOURCE rc = { 0 };
        HRESULT hr = S_OK;

        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        _D3D11_SecondaryContextSync.Lock();
        hr = _D3D11_SecondaryContext->Map(self->buffer, NULL, D3D11_MAP_WRITE_DISCARD, 0, &rc);
        _D3D11_SecondaryContextSync.Unlock();

        if (SUCCEEDED(hr) && rc.pData)
        {
            self->isMapped = true;
            ptr = rc.pData;
        }
        #else
        if (!self->mappedData)
            self->mappedData = ::malloc(self->size);

        self->isMapped = true;
        ptr = ((uint8*)self->mappedData) + offset;
        #endif

        CHECK_HR(hr)
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE rc = { 0 };
        DX11Command cmd = { DX11Command::MAP, { uint64(self->buffer), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64(&rc) } };

        DVASSERT(self->usage != USAGE_STATICDRAW);
        DVASSERT(!self->isMapped);
        ExecDX11(&cmd, 1);
        CHECK_HR(cmd.retval)

        if (rc.pData)
        {
            ptr = rc.pData;
            self->isMapped = true;
        }
    }

    return ((uint8*)ptr) + offset;
}

//------------------------------------------------------------------------------

static void
dx11_VertexBuffer_Unmap(Handle vb)
{
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get(vb);

    DVASSERT(self->usage != USAGE_STATICDRAW);
    DVASSERT(self->isMapped);

    if (self->usage == USAGE_DYNAMICDRAW)
    {
        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        _D3D11_SecondaryContextSync.Lock();
        _D3D11_SecondaryContext->Unmap(self->buffer, 0);
        _D3D11_SecondaryContextSync.Unlock();
        #else
        self->updatePending = true;
        #endif
        self->isMapped = false;
    }
    else
    {
        DX11Command cmd = { DX11Command::UNMAP, { uint64(self->buffer), 0 } };

        ExecDX11(&cmd, 1);
        self->isMapped = false;
    }
}

//------------------------------------------------------------------------------

namespace VertexBufferDX11
{
void Init(uint32 maxCount)
{
    VertexBufferDX11Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_VertexBuffer_Create = &dx11_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete = &dx11_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update = &dx11_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map = &dx11_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap = &dx11_VertexBuffer_Unmap;
}

void SetToRHI(Handle vbh, unsigned stream_i, unsigned offset, unsigned stride, ID3D11DeviceContext* context)
{
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get(vbh);
    ID3D11Buffer* vb[1] = { self->buffer };
    UINT vb_offset[1] = { offset };
    UINT vb_stride[1] = { stride };

    #if !RHI_DX11__USE_DEFERRED_CONTEXTS
    if (self->updatePending)
    {
        D3D11_MAPPED_SUBRESOURCE rc = { 0 };
        HRESULT hr;

        hr = context->Map(self->buffer, NULL, D3D11_MAP_WRITE_DISCARD, 0, &rc);
        CHECK_HR(hr)

        if (SUCCEEDED(hr) && rc.pData)
        {
            memcpy(rc.pData, self->mappedData, self->size);
            context->Unmap(self->buffer, 0);
        }
        self->updatePending = true;
    }
    #endif
    ///    DVASSERT(!self->isMapped);
    context->IASetVertexBuffers(stream_i, 1, vb, vb_stride, vb_offset);
}
}

} // namespace rhi
