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
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;

    #include "_metal.h"

#if !defined(TARGET_IPHONE_SIMULATOR)

namespace rhi
{
//==============================================================================

class
QueryBufferMetal_t
{
public:
    QueryBufferMetal_t();
    ~QueryBufferMetal_t();

    uint32 maxObjectCount;
    id<MTLBuffer> uid;
};

typedef ResourcePool<QueryBufferMetal_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false> QueryBufferMetalPool;
RHI_IMPL_POOL(QueryBufferMetal_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false);

//==============================================================================

QueryBufferMetal_t::QueryBufferMetal_t()
    : maxObjectCount(0)
{
}

//------------------------------------------------------------------------------

QueryBufferMetal_t::~QueryBufferMetal_t()
{
}

static Handle
metal_QueryBuffer_Create(uint32 maxObjectCount)
{
    Handle handle = QueryBufferMetalPool::Alloc();
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get(handle);

    if (buf)
    {
        int sz = maxObjectCount * QueryBUfferElemeentAlign;
        id<MTLBuffer> uid = [_Metal_Device newBufferWithLength:sz options:MTLResourceOptionCPUCacheModeDefault];

        buf->uid = uid;
        buf->maxObjectCount = maxObjectCount;

        memset([uid contents], 0x00, sz);
    }

    return handle;
}

static void
metal_QueryBuffer_Reset(Handle handle)
{
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get(handle);

    if (buf)
    {
        memset([buf->uid contents], 0x00, buf->maxObjectCount * QueryBUfferElemeentAlign);
    }
}

static void
metal_QueryBuffer_Delete(Handle handle)
{
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get(handle);

    if (buf)
    {
        [buf->uid release];
        buf->uid = nil;
    }

    QueryBufferMetalPool::Free(handle);
}

static bool
metal_QueryBuffer_IsReady(Handle handle, uint32 objectIndex)
{
    return true;
    /*
    bool                ready = false;
    QueryBufferMetal_t* buf   = QueryBufferMetalPool::Get( handle );

    if( buf  &&  objectIndex < buf->maxObjectCount )
    {
    }

    return ready;
*/
}

static int
metal_QueryBuffer_Value(Handle handle, uint32 objectIndex)
{
    int value = 0;
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get(handle);

    if (buf && objectIndex < buf->maxObjectCount)
    {
        uint8* data = (uint8*)([buf->uid contents]) + objectIndex * QueryBUfferElemeentAlign;

        value = *((uint32*)data);
    }

    return value;
}

namespace QueryBufferMetal
{
id<MTLBuffer> GetBuffer(Handle handle)
{
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get(handle);

    return (buf) ? buf->uid : nil;
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_QueryBuffer_Create = &metal_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset = &metal_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete = &metal_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady = &metal_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value = &metal_QueryBuffer_Value;
}
}

//==============================================================================
} // namespace rhi

#endif //#if !defined(TARGET_IPHONE_SIMULATOR)