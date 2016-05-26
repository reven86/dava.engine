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

static Handle
dx9_PerfQuerySet_Create(uint32 maxTimestampCount)
{
    return InvalidHandle;
}

static void
dx9_PerfQuerySet_Delete(Handle handle)
{
}

static void
dx9_PerfQuerySet_Reset(Handle handle)
{
}

static void
dx9_PerfQuerySet_SetCurrent(Handle handle)
{
}

static void
dx9_PerfQuerySet_GetStatus(Handle handle, bool* IsReady, bool* isValid)
{
    *IsReady = false;
    *isValid = false;
}

static bool
dx9_PerfQuerySet_GetFreq(Handle handle, uint64* freq)
{
    return false;
}

static bool
dx9_PerfQuerySet_GetTimestamp(Handle handle, uint32 timestampIndex, uint64* time)
{
    return false;
}

static bool
dx9_PerfQuerySet_GetFrameTimestamps(Handle handle, uint64* t0, uint64* t1)
{
    return false;
}

namespace PerfQuerySetDX9
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuerySet_Create = &dx9_PerfQuerySet_Create;
    dispatch->impl_PerfQuerySet_Delete = &dx9_PerfQuerySet_Delete;
    dispatch->impl_PerfQuerySet_Reset = &dx9_PerfQuerySet_Reset;
    dispatch->impl_PerfQuerySet_SetCurrent = &dx9_PerfQuerySet_SetCurrent;
    dispatch->impl_PerfQuerySet_GetStatus = &dx9_PerfQuerySet_GetStatus;
    dispatch->impl_PerfQuerySet_GetFreq = &dx9_PerfQuerySet_GetFreq;
    dispatch->impl_PerfQuerySet_GetTimestamp = &dx9_PerfQuerySet_GetTimestamp;
    dispatch->impl_PerfQuerySet_GetFrameTimestamps = &dx9_PerfQuerySet_GetFrameTimestamps;
}
}

//==============================================================================
} // namespace rhi
