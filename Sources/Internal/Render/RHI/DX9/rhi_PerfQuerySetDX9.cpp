/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX9.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
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
