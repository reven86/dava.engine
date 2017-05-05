#include "rhi_NullRenderer.h"
#include "../rhi_Type.h"

#include "../Common/rhi_BackendImpl.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"

namespace rhi
{
using QueryBufferNullPool = ResourcePool<ResourceNull_t, RESOURCE_QUERY_BUFFER, NullResourceDescriptor>;
RHI_IMPL_POOL(ResourceNull_t, RESOURCE_QUERY_BUFFER, NullResourceDescriptor, false);

//////////////////////////////////////////////////////////////////////////

Handle null_QueryBuffer_Create(unsigned maxObjectCount)
{
    return QueryBufferNullPool::Alloc();
}

void null_QueryBuffer_Reset(Handle)
{
}

void null_QueryBuffer_Delete(Handle h)
{
    QueryBufferNullPool::Free(h);
}

bool null_QueryBuffer_IsReady(Handle)
{
    return true;
}

bool null_QueryBuffer_ObjectIsReady(Handle, uint32)
{
    return true;
}

int null_QueryBuffer_Value(Handle, uint32)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////

namespace QueryBufferNull
{
void Init(uint32 maxCount)
{
    QueryBufferNullPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_QueryBuffer_Create = null_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset = null_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete = null_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady = null_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_ObjectIsReady = null_QueryBuffer_ObjectIsReady;
    dispatch->impl_QueryBuffer_Value = null_QueryBuffer_Value;
}
}
} //ns rhi