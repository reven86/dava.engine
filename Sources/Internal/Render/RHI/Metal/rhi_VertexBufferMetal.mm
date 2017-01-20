#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "mem_BufferAllocator.h"
    #include "_metal.h"

#if !(TARGET_IPHONE_SIMULATOR == 1)

namespace rhi
{
struct
VertexBufferMetal_t
{
    VertexBufferMetal_t()
        : size(0)
        , data(0)
        , uid(nil)
    {
    }
    ~VertexBufferMetal_t()
    {
    }

    uint32 size;
    void* data;
    id<MTLBuffer> uid;
    MetalBufferAllocator::Block block;
};

typedef ResourcePool<VertexBufferMetal_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, false> VertexBufferMetalPool;
RHI_IMPL_POOL(VertexBufferMetal_t, RESOURCE_VERTEX_BUFFER, VertexBuffer::Descriptor, false);

static MetalBufferAllocator _VB_Pool("VB", 4 * 1024 * 1024, 1024);

//==============================================================================

static Handle
metal_VertexBuffer_Create(const VertexBuffer::Descriptor& desc)
{
    Handle handle = InvalidHandle;

    if (desc.usage == USAGE_DYNAMICDRAW)
    {
        id<MTLBuffer> uid = (desc.initialData)
        ?
        [_Metal_Device newBufferWithBytes:desc.initialData
                                   length:desc.size
                                  options:MTLResourceOptionCPUCacheModeDefault]
        :
        [_Metal_Device newBufferWithLength:desc.size
                                   options:MTLResourceOptionCPUCacheModeDefault];

        if (uid)
        {
            handle = VertexBufferMetalPool::Alloc();
            VertexBufferMetal_t* vb = VertexBufferMetalPool::Get(handle);
            //DAVA::Logger::Info( "vb-create-dynamic %i  %p sz= %u", RHI_HANDLE_INDEX(handle), [uid contents], desc.size );

            vb->data = [uid contents];
            vb->size = desc.size;
            vb->uid = uid;

            [vb->uid retain];
        }
    }
    else
    {
        MetalBufferAllocator::Block block;

        if (_VB_Pool.alloc(desc.size, &block))
        {
            handle = VertexBufferMetalPool::Alloc();
            VertexBufferMetal_t* vb = VertexBufferMetalPool::Get(handle);

            vb->block = block;
            vb->size = desc.size;
            vb->uid = nil;

            if (desc.initialData)
                block.Update(desc.initialData);

            //DAVA::Logger::Info( "vb-create %i  sz= %u", RHI_HANDLE_INDEX(handle), desc.size );
            //_VB_Pool.dump_stats();
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Delete(Handle vb, bool)
{
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get(vb);

    if (self)
    {
        //DAVA::Logger::Info( "vb-del %i  %p sz= %u", RHI_HANDLE_INDEX(vb), [self->uid contents], self->size );
        if (self->uid)
        {
            [self->uid release];
            [self->uid release];
            self->uid = nil;
            self->data = nullptr;
        }
        else
        {
            _VB_Pool.free(self->block);
            self->data = nullptr;
            self->block.uid = nil;
            self->block.base = 0;
            //_VB_Pool.dump_stats();
        }

        VertexBufferMetalPool::Free(vb);
    }
}

//------------------------------------------------------------------------------

static bool
metal_VertexBuffer_Update(Handle vb, const void* data, uint32 offset, uint32 size)
{
    bool success = false;
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get(vb);

    //    if (!self->data)
    //        self->data = [self->uid contents];

    if (offset + size <= self->size)
    {
        if (self->uid)
            memcpy(((uint8*)self->data) + offset, data, size);
        else
            self->block.Update(data);
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

static void*
metal_VertexBuffer_Map(Handle vb, uint32 offset, uint32 size)
{
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get(vb);

    //    if (!self->data)
    //        self->data = [self->uid contents];

    DVASSERT(self->data);

    return (offset + size <= self->size) ? ((uint8*)self->data) + offset : 0;
}

//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Unmap(Handle vb)
{
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get(vb);

    DVASSERT(self->data);
    //    self->data = nullptr;
}

//------------------------------------------------------------------------------

static bool
metal_VertexBuffer_NeedRestore(Handle vb)
{
    //    VertexBuffer_t* self = VertexBufferMetalPool::Get( vb );
    //    return self->NeedRestore();
    return false;
}

namespace VertexBufferMetal
{
void Init(uint32 maxCount)
{
    VertexBufferMetalPool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_VertexBuffer_Create = &metal_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete = &metal_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update = &metal_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map = &metal_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap = &metal_VertexBuffer_Unmap;
    dispatch->impl_VertexBuffer_NeedRestore = &metal_VertexBuffer_NeedRestore;
}

id<MTLBuffer>
GetBuffer(Handle ib, unsigned* base)
{
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get(ib);
    id<MTLBuffer> uid;

    if (self->uid)
    {
        uid = self->uid;
        *base = 0;
    }
    else
    {
        uid = self->block.uid;
        *base = self->block.base;
    }

    return uid;
}
}

} // namespace rhi

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
