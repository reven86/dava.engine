#include "rhi_DX11.h"

namespace rhi
{
struct SyncObjectDX11_t
{
    uint32 frame = 0;
    uint32 is_signaled : 1;
    uint32 is_used : 1;
    uint32 pad : 30;
};
using SyncObjectPoolDX11 = ResourcePool<SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false>;
RHI_IMPL_POOL(SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

static DAVA::Mutex _DX11_SyncObjectsSync;

static Handle dx11_SyncObject_Create()
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);

    Handle handle = SyncObjectPoolDX11::Alloc();
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(handle);
    sync->is_signaled = false;
    sync->is_used = false;
    return handle;
}

static void dx11_SyncObject_Delete(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);
    SyncObjectPoolDX11::Free(obj);
}

static bool dx11_SyncObject_IsSignaled(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);
    if (!SyncObjectPoolDX11::IsAlive(obj))
        return true;

    bool signaled = false;
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

void SyncObjectDX11::SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_SyncObject_Create = &dx11_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &dx11_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &dx11_SyncObject_IsSignaled;
}

bool SyncObjectDX11::IsAlive(Handle handle)
{
    return SyncObjectPoolDX11::IsAlive(handle);
}

void SyncObjectDX11::SetProperties(Handle handle, DAVA::uint32 frameNumber, bool signaled, bool used)
{
    SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(handle);
    s->frame = frameNumber;
    s->is_signaled = signaled;
    s->is_used = used;
}

void SyncObjectDX11::SetSignaledAndUsedProperties(Handle handle, bool signaled, bool used)
{
    SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(handle);
    s->is_signaled = signaled;
    s->is_used = used;
}

void SyncObjectDX11::SetFrameNumberAndSignaledProperties(Handle handle, DAVA::uint32 frameNumber, bool signaled)
{
    SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(handle);
    s->is_signaled = signaled;
    s->frame = frameNumber;
}

void SyncObjectDX11::CheckFrameAndSignalUsed(DAVA::uint32 frameNumber)
{
    DAVA::LockGuard<DAVA::Mutex> lock(_DX11_SyncObjectsSync);
    for (SyncObjectPoolDX11::Iterator s = SyncObjectPoolDX11::Begin(), s_end = SyncObjectPoolDX11::End(); s != s_end; ++s)
    {
        if (s->is_used && (frameNumber - s->frame >= 2))
            s->is_signaled = true;
    }
}
}
