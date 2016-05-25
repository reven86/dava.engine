#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_metal.h"
    #include "mem_RemoteHeap.h"
    #include "mem_BufferAllocator.h"

#if !(TARGET_IPHONE_SIMULATOR == 1)

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
    BufferAllocator::Block block;
    MTLIndexType type;
};

typedef ResourcePool<IndexBufferMetal_t, RESOURCE_INDEX_BUFFER, IndexBuffer::Descriptor, false> IndexBufferMetalPool;
RHI_IMPL_POOL(IndexBufferMetal_t, RESOURCE_INDEX_BUFFER, IndexBuffer::Descriptor, false);

static bool
_NonZero(const void* data, unsigned data_sz)
{
    static unsigned zero_sz = 0;
    static void* zero = nullptr;

    if (data_sz > zero_sz)
    {
        zero = realloc(zero, data_sz);
        zero_sz = data_sz;
        memset(zero, 0, zero_sz);
    }

    return memcmp(data, zero, data_sz) != 0;
}

static void
_CheckAllIndexBuffers()
{
    for (IndexBufferMetalPool::Iterator i = IndexBufferMetalPool::Begin(), i_end = IndexBufferMetalPool::End(); i != i_end; ++i)
    {
        if (!_NonZero([i->uid contents], i->size))
        {
            DAVA::Logger::Info("ib-lost  %p sz= %u", [i->uid contents], i->size);
            //            DVASSERT(!"kaboom!!!");
        }
    }
}

static BufferAllocator _IB_Pool("IB", 2 * 1024 * 1024, 256);

//==============================================================================

static Handle
metal_IndexBuffer_Create(const IndexBuffer::Descriptor& desc)
{
    Handle handle = InvalidHandle;

    if (desc.usage == USAGE_DYNAMICDRAW)
    {
        id<MTLBuffer> uid = (desc.initialData)
        ?
        [_Metal_Device newBufferWithBytes:desc.initialData length:desc.size options:MTLResourceOptionCPUCacheModeDefault]
        :
        [_Metal_Device newBufferWithLength:desc.size options:MTLResourceOptionCPUCacheModeDefault];

        if (uid)
        {
            handle = IndexBufferMetalPool::Alloc();
            IndexBufferMetal_t* ib = IndexBufferMetalPool::Get(handle);
            //DAVA::Logger::Info( "ib-create-dynamic %i  %p sz= %u", RHI_HANDLE_INDEX(handle), [uid contents], desc.size );

            ib->data = [uid contents];
            ib->size = desc.size;
            ib->uid = uid;
            ib->type = (desc.indexSize == INDEX_SIZE_32BIT) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;

            [ib->uid setPurgeableState:MTLPurgeableStateNonVolatile];
        }
    }
    else
    {
        BufferAllocator::Block block;

        if (_IB_Pool.alloc(desc.size, &block))
        {
            //DAVA::Logger::Info( "ib-create %i  sz= %u", RHI_HANDLE_INDEX(handle), desc.size );
            //_IB_Pool.dump_stats();
            handle = IndexBufferMetalPool::Alloc();
            IndexBufferMetal_t* ib = IndexBufferMetalPool::Get(handle);

            ib->block = block;
            ib->size = desc.size;
            ib->type = (desc.indexSize == INDEX_SIZE_32BIT) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
            ib->data = block.ptr;
            ib->uid = nil;

            if (desc.initialData)
                memcpy(block.ptr, desc.initialData, desc.size);
        }
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
        //DAVA::Logger::Info( "ib-del %i  %p sz= %u", RHI_HANDLE_INDEX(ib), [self->uid contents], self->size );
        if (self->uid)
        {
            [self->uid setPurgeableState:MTLPurgeableStateEmpty];
            [self->uid release];
            self->uid = nil;
            self->data = nullptr;
        }
        else
        {
            _IB_Pool.free(self->block);
            self->data = nullptr;
            self->block.ptr = nullptr;
            self->block.buffer = nil;
            self->block.base = 0;
            //_IB_Pool.dump_stats();
        }

        IndexBufferMetalPool::Free(ib);
    }
    //_CheckAllIndexBuffers();
}

//------------------------------------------------------------------------------

static bool
metal_IndexBuffer_Update(Handle ib, const void* data, unsigned offset, unsigned size)
{
    bool success = false;
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    //    if (!self->data)
    //        self->data = [self->uid contents];

    if (offset + size <= self->size)
    {
        memcpy(((uint8*)self->data) + offset, data, size);
        success = true;
    }

    //DVASSERT(_NonZero( self->data, self->size ));
    //   self->data = nullptr;

    return success;
}

//------------------------------------------------------------------------------

static void*
metal_IndexBuffer_Map(Handle ib, unsigned offset, unsigned size)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    //    if (!self->data)
    //        self->data = [self->uid contents];

    DVASSERT(self->data);

    return (offset + size <= self->size) ? ((uint8*)self->data) + offset : 0;
}

//------------------------------------------------------------------------------

static void
metal_IndexBuffer_Unmap(Handle ib)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);

    //DVASSERT(_NonZero( self->data, self->size ));
    DVASSERT(self->data);
    //    self->data = nullptr;
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
GetBuffer(Handle ib, unsigned* base)
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get(ib);
    id<MTLBuffer> uid;

    if (self->uid)
    {
        uid = self->uid;
        *base = 0;
    }
    else
    {
        uid = self->block.buffer;
        *base = self->block.base;
    }

    return uid;
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

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
