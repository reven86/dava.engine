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

namespace rhi
{
//==============================================================================

struct
IndexBufferMetal_t
{
public:
    IndexBufferMetal_t()
        : size(0)
        , data(0)
        , uid(nil)
    {
    }

    unsigned size;
    void* data;
    id<MTLBuffer> uid;
    MTLIndexType type;
};

typedef ResourcePool<IndexBufferMetal_t, RESOURCE_INDEX_BUFFER, IndexBuffer::Descriptor, false> IndexBufferMetalPool;
RHI_IMPL_POOL(IndexBufferMetal_t, RESOURCE_INDEX_BUFFER, IndexBuffer::Descriptor, false);

//==============================================================================

static Handle
metal_IndexBuffer_Create(const IndexBuffer::Descriptor& desc)
{
    Handle handle = InvalidHandle;
    id<MTLBuffer> uid = (desc.initialData) ? [_Metal_Device newBufferWithBytes:desc.initialData length:desc.size options:MTLResourceOptionCPUCacheModeDefault] : [_Metal_Device newBufferWithLength:desc.size options:MTLResourceOptionCPUCacheModeDefault];

    if (uid)
    {
        handle = IndexBufferMetalPool::Alloc();
        IndexBufferMetal_t* ib = IndexBufferMetalPool::Get(handle);

        ib->data = [uid contents];
        ib->size = desc.size;
        ib->uid = uid;
        ib->type = (desc.indexSize == INDEX_SIZE_32BIT) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
metal_IndexBuffer_Delete(Handle ib)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    if (self)
    {
        IndexBufferMetalPool::Free(ib);
    }
}

//------------------------------------------------------------------------------

static bool
metal_IndexBuffer_Update(Handle ib, const void* data, unsigned offset, unsigned size)
{
    bool success = false;
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    if (offset + size <= self->size)
    {
        memcpy(((uint8*)self->data) + offset, data, size);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

static void*
metal_IndexBuffer_Map(Handle ib, unsigned offset, unsigned size)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    DVASSERT(self->data);

    return (offset + size <= self->size) ? ((uint8*)self->data) + offset : 0;
}

//------------------------------------------------------------------------------

static void
metal_IndexBuffer_Unmap(Handle ib)
{
    // do nothing
}

//------------------------------------------------------------------------------

namespace IndexBufferMetal
{
void Init(uint32 maxCount)
{
    IndexBufferMetalPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_IndexBuffer_Create = &metal_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete = &metal_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update = &metal_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map = &metal_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap = &metal_IndexBuffer_Unmap;
}

id<MTLBuffer>
GetBuffer(Handle ib)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    return (self) ? self->uid : nil;
}

MTLIndexType
GetType(Handle ib)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    return self->type;
}

} // namespace IndexBufferGLES

//==============================================================================
} // namespace rhi
