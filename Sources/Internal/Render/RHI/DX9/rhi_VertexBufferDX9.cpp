#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "rhi_DX9.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
using DAVA::Logger;

#include "_dx9.h"

namespace rhi
{
//==============================================================================

class
VertexBufferDX9_t
: public ResourceImpl<VertexBufferDX9_t, VertexBuffer::Descriptor>
{
public:
    VertexBufferDX9_t();
    ~VertexBufferDX9_t();

    bool Create(const VertexBuffer::Descriptor& desc, bool force_immediate = false);
    void Destroy(bool force_immediate = false);

    unsigned size = 0;
    IDirect3DVertexBuffer9* buffer = nullptr;
    DAVA::Vector<uint8> mappedData;
    unsigned isMapped : 1;
};

RHI_IMPL_RESOURCE(VertexBufferDX9_t, VertexBuffer::Descriptor)

typedef ResourcePool<VertexBufferDX9_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, true> VertexBufferDX9Pool;
RHI_IMPL_POOL(VertexBufferDX9_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, true);

VertexBufferDX9_t::VertexBufferDX9_t()
    : size(0)
    , buffer(nullptr)
    , isMapped(false)
{
}

//------------------------------------------------------------------------------

VertexBufferDX9_t::~VertexBufferDX9_t()
{
}

//------------------------------------------------------------------------------

bool VertexBufferDX9_t::Create(const VertexBuffer::Descriptor& desc, bool force_immediate)
{
    DVASSERT(desc.size);
    bool success = false;
    UpdateCreationDesc(desc);

    if (desc.size)
    {
        DWORD usage = D3DUSAGE_WRITEONLY;

        switch (desc.usage)
        {
        case USAGE_DEFAULT:
            usage = D3DUSAGE_WRITEONLY;
            break;
        case USAGE_STATICDRAW:
            usage = D3DUSAGE_WRITEONLY;
            break;
        case USAGE_DYNAMICDRAW:
            usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
            break;
        }

        uint32 cmd_cnt = 2;
        DX9Command cmd[2] =
        {
          { DX9Command::CREATE_VERTEX_BUFFER, { desc.size, usage, 0, D3DPOOL_DEFAULT, uint64_t(&buffer), NULL } },
          { DX9Command::UPDATE_VERTEX_BUFFER, { uint64_t(&buffer), uint64_t(desc.initialData), desc.size } }
        };

        if (!desc.initialData)
        {
            cmd[1].func = DX9Command::NOP;
            cmd_cnt = 1;
        }

        ExecDX9(cmd, cmd_cnt, force_immediate);

        if (SUCCEEDED(cmd[0].retval))
        {
            size = desc.size;
            isMapped = false;

            success = true;
        }
        else
        {
            Logger::Error("FAILED to create vertex-buffer:\n%s\n", D3D9ErrorText(cmd[0].retval));
        }
    }

    return success;
}

//------------------------------------------------------------------------------

void VertexBufferDX9_t::Destroy(bool force_immediate)
{
    if (buffer)
    {
        DX9Command cmd[] = { DX9Command::RELEASE, { reinterpret_cast<uint64_t>(buffer) } };
        ExecDX9(cmd, countof(cmd), force_immediate);
        buffer = nullptr;
    }

    size = 0;
}

//==============================================================================

//------------------------------------------------------------------------------

static Handle
dx9_VertexBuffer_Create(const VertexBuffer::Descriptor& desc)
{
    CommandBufferDX9::BlockNonRenderThreads();

    Handle handle = VertexBufferDX9Pool::Alloc();
    VertexBufferDX9_t* vb = VertexBufferDX9Pool::Get(handle);

    if (vb->Create(desc) == false)
    {
        VertexBufferDX9Pool::Free(handle);
        handle = InvalidHandle;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
dx9_VertexBuffer_Delete(Handle vb)
{
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);
    self->MarkRestored();
    self->Destroy();
    VertexBufferDX9Pool::Free(vb);
}

//------------------------------------------------------------------------------

static bool
dx9_VertexBuffer_Update(Handle vb, const void* data, unsigned offset, unsigned size)
{
    bool success = false;
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);

    DVASSERT(!self->isMapped);

    if (offset + size <= self->size)
    {
        void* ptr = nullptr;
        DX9Command cmd1 = { DX9Command::LOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)), offset, size, uint64_t(&ptr), 0 } };

        ExecDX9(&cmd1, 1);
        if (SUCCEEDED(cmd1.retval))
        {
            memcpy(ptr, data, size);

            DX9Command cmd2 = { DX9Command::UNLOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)) } };

            ExecDX9(&cmd2, 1);
            success = true;

            self->MarkRestored();
        }
    }

    return success;
}

//------------------------------------------------------------------------------

static void*
dx9_VertexBuffer_Map(Handle vb, unsigned offset, unsigned size)
{
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);

    DVASSERT(self->CreationDesc().usage != Usage::USAGE_STATICDRAW);

    if (self->mappedData.size() < self->size)
    {
        self->mappedData.resize(self->size);
    }

    self->isMapped = true;
    return self->mappedData.data() + offset;
}

//------------------------------------------------------------------------------

static void
dx9_VertexBuffer_Unmap(Handle vb)
{
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);

    DX9Command cmd = { DX9Command::UPDATE_VERTEX_BUFFER, { uint64_t(&self->buffer), uint64_t(self->mappedData.data()), self->mappedData.size() } };
    ExecDX9(&cmd, 1);
    DVASSERT(SUCCEEDED(cmd.retval));

    self->isMapped = false;
    self->MarkRestored();
}

//------------------------------------------------------------------------------

static bool
dx9_VertexBuffer_NeedRestore(Handle vb)
{
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);

    return self->NeedRestore();
}

//------------------------------------------------------------------------------

namespace VertexBufferDX9
{
void Init(uint32 maxCount)
{
    VertexBufferDX9Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_VertexBuffer_Create = &dx9_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete = &dx9_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update = &dx9_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map = &dx9_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap = &dx9_VertexBuffer_Unmap;
    dispatch->impl_VertexBuffer_NeedRestore = &dx9_VertexBuffer_NeedRestore;
}

void SetToRHI(Handle vb, unsigned stream_i, unsigned offset, unsigned stride)
{
    VertexBufferDX9_t* self = VertexBufferDX9Pool::Get(vb);
    HRESULT hr = _D3D9_Device->SetStreamSource(stream_i, self->buffer, offset, stride);

    DVASSERT(!self->isMapped);

    if (FAILED(hr))
        Logger::Error("SetStreamSource failed:\n%s\n", D3D9ErrorText(hr));
}

void ReleaseAll()
{
    VertexBufferDX9Pool::Unlock();
    for (VertexBufferDX9Pool::Iterator b = VertexBufferDX9Pool::Begin(), b_end = VertexBufferDX9Pool::End(); b != b_end; ++b)
    {
        b->Destroy(true);
    }
    VertexBufferDX9Pool::Unlock();
}

void ReCreateAll()
{
    VertexBufferDX9Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return VertexBufferDX9Pool::PendingRestoreCount();
}
}

} // namespace rhi
