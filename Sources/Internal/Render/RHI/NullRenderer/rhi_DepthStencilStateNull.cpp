#include "rhi_NullRenderer.h"
#include "../rhi_Type.h"

#include "../Common/rhi_BackendImpl.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"

namespace rhi
{
using DepthStencilStateNullPool = ResourcePool<ResourceNull_t, RESOURCE_DEPTHSTENCIL_STATE, NullResourceDescriptor>;
RHI_IMPL_POOL(ResourceNull_t, RESOURCE_DEPTHSTENCIL_STATE, NullResourceDescriptor, false);

//////////////////////////////////////////////////////////////////////////

Handle null_DepthStencilState_Create(const DepthStencilState::Descriptor&)
{
    return DepthStencilStateNullPool::Alloc();
}

void null_DepthStencilState_Delete(Handle h)
{
    DepthStencilStateNullPool::Free(h);
}

//////////////////////////////////////////////////////////////////////////

namespace DepthStencilStateNull
{
void Init(uint32 maxCount)
{
    DepthStencilStateNullPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_DepthStencilState_Create = null_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = null_DepthStencilState_Delete;
}
}
} //ns rhi