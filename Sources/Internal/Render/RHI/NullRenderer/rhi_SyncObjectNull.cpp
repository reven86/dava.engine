#include "rhi_NullRenderer.h"
#include "../rhi_Type.h"

#include "../Common/rhi_BackendImpl.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"

namespace rhi
{
using SyncObjectNullPool = ResourcePool<ResourceNull_t, RESOURCE_SYNC_OBJECT, NullResourceDescriptor>;
RHI_IMPL_POOL(ResourceNull_t, RESOURCE_SYNC_OBJECT, NullResourceDescriptor, false);

//////////////////////////////////////////////////////////////////////////

Handle null_SyncObject_Create()
{
    return SyncObjectNullPool::Alloc();
}

void null_SyncObject_Delete(Handle h)
{
    SyncObjectNullPool::Free(h);
}

bool null_SyncObject_IsSignaled(Handle)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////

namespace SyncObjectNull
{
void Init(uint32 maxCount)
{
    SyncObjectNullPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_SyncObject_Create = null_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = null_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = null_SyncObject_IsSignaled;
}
}
} //ns rhi