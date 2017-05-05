#include "rhi_NullRenderer.h"
#include "../rhi_Type.h"

#include "../Common/rhi_BackendImpl.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"

namespace rhi
{
using SamplerStateNullPool = ResourcePool<ResourceNull_t, RESOURCE_SAMPLER_STATE, NullResourceDescriptor>;
RHI_IMPL_POOL(ResourceNull_t, RESOURCE_SAMPLER_STATE, NullResourceDescriptor, false);

//////////////////////////////////////////////////////////////////////////

Handle null_SamplerState_Create(const SamplerState::Descriptor&)
{
    return SamplerStateNullPool::Alloc();
}

void null_SamplerState_Delete(Handle h)
{
    SamplerStateNullPool::Free(h);
}

//////////////////////////////////////////////////////////////////////////

namespace SamplerStateNull
{
void Init(uint32 maxCount)
{
    SamplerStateNullPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_SamplerState_Create = null_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = null_SamplerState_Delete;
}
}
} //ns rhi