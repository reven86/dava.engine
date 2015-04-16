
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
                      : size(0),
                        data(0),
                        uid(nil)
                    {}


    unsigned        size;
    void*           data;
    id<MTLBuffer>   uid;
};

typedef Pool<IndexBufferMetal_t,RESOURCE_INDEX_BUFFER>    IndexBufferMetalPool;
RHI_IMPL_POOL(IndexBufferMetal_t,RESOURCE_INDEX_BUFFER);


//==============================================================================

static Handle
metal_IndexBuffer_Create( uint32 size, uint32 options )
{
    Handle          handle  = InvalidHandle;
    id<MTLBuffer>   uid     = [_Metal_Device newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];

    if( uid )
    {
        handle = IndexBufferMetalPool::Alloc();
        IndexBufferMetal_t*    ib = IndexBufferMetalPool::Get( handle );

        ib->data   = [uid contents];
        ib->size   = size;
        ib->uid    = uid;

    }

    return handle;
}


//------------------------------------------------------------------------------

static void
metal_IndexBuffer_Delete( Handle ib )
{
    IndexBufferMetal_t*    self = IndexBufferMetalPool::Get( ib );

    if( self )
    {
        delete self;
    }
}


//------------------------------------------------------------------------------
    
static bool
metal_IndexBuffer_Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferMetal_t* self    = IndexBufferMetalPool::Get( ib );

    if( offset+size <= self->size )
    {
        memcpy( ((uint8*)self->data)+offset, data, size );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
metal_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get( ib );

    DVASSERT(self->data);
    
    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

static void
metal_IndexBuffer_Unmap( Handle ib )
{
    // do nothing
}


//------------------------------------------------------------------------------

namespace IndexBufferMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create  = &metal_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete  = &metal_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update  = &metal_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map     = &metal_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap   = &metal_IndexBuffer_Unmap;
}
    
id<MTLBuffer> 
GetBuffer( Handle ib )
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get( ib );

    return (self)  ? self->uid  : nil;
}


} // namespace IndexBufferGLES

//==============================================================================
} // namespace rhi

