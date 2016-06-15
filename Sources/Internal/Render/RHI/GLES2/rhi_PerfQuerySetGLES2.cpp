#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_gl.h"

namespace rhi
{
//==============================================================================

static Handle
gles2_PerfQuerySet_Create(uint32 maxTimestampCount)
{
    return InvalidHandle;
}

static void
gles2_PerfQuerySet_Delete(Handle handle)
{
}

static void
gles2_PerfQuerySet_Reset(Handle handle)
{
}

static void
gles2_PerfQuerySet_SetCurrent(Handle handle)
{
}

static void
gles2_PerfQuerySet_GetStatus(Handle handle, bool* IsReady, bool* isValid)
{
    *IsReady = false;
    *isValid = false;
}

static bool
gles2_PerfQuerySet_GetFreq(Handle handle, uint64* freq)
{
    return false;
}

static bool
gles2_PerfQuerySet_GetTimestamp(Handle handle, uint32 timestampIndex, uint64* time)
{
    return false;
}

static bool
gles2_PerfQuerySet_GetFrameTimestamps(Handle handle, uint64* t0, uint64* t1)
{
    return false;
}

namespace PerfQuerySetGLES2
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuerySet_Create = &gles2_PerfQuerySet_Create;
    dispatch->impl_PerfQuerySet_Delete = &gles2_PerfQuerySet_Delete;
    dispatch->impl_PerfQuerySet_Reset = &gles2_PerfQuerySet_Reset;
    dispatch->impl_PerfQuerySet_SetCurrent = &gles2_PerfQuerySet_SetCurrent;
    dispatch->impl_PerfQuerySet_GetStatus = &gles2_PerfQuerySet_GetStatus;
    dispatch->impl_PerfQuerySet_GetFreq = &gles2_PerfQuerySet_GetFreq;
    dispatch->impl_PerfQuerySet_GetTimestamp = &gles2_PerfQuerySet_GetTimestamp;
    dispatch->impl_PerfQuerySet_GetFrameTimestamps = &gles2_PerfQuerySet_GetFrameTimestamps;
}
}

//==============================================================================
} // namespace rhi
