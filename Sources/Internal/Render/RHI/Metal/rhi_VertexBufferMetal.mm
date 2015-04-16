

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_metal.h"


namespace rhi
{

struct
VertexBufferMetal_t
{
                    VertexBufferMetal_t()
                      : size(0),
                        data(0),
                        uid(nil)
                    {}
                    ~VertexBufferMetal_t()
                    {}


    uint32          size;
    void*           data;
    id<MTLBuffer>   uid;
};

typedef Pool<VertexBufferMetal_t,RESOURCE_VERTEX_BUFFER> VertexBufferMetalPool;
RHI_IMPL_POOL(VertexBufferMetal_t,RESOURCE_VERTEX_BUFFER);


//==============================================================================


static Handle
metal_VertexBuffer_Create( uint32 size, uint32 options )
{
    Handle          handle  = InvalidHandle;
    id<MTLBuffer>   uid     = [_Metal_Device newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];

    if( uid )
    {
        handle = VertexBufferMetalPool::Alloc();
        VertexBufferMetal_t*    vb = VertexBufferMetalPool::Get( handle );

        vb->data   = [uid contents];
        vb->size   = size;
        vb->uid    = uid;

    }

    return handle;
}


//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Delete( Handle vb )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    if( self )
    {
    }
}


//------------------------------------------------------------------------------
    
static bool
metal_VertexBuffer_Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    bool                    success = false;
    VertexBufferMetal_t*    self    = VertexBufferMetalPool::Get( vb );

    if( offset+size <= self->size )
    {
        memcpy( ((uint8*)self->data)+offset, data, size );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
metal_VertexBuffer_Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    DVASSERT(self->data);
    
    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Unmap( Handle vb )
{
    // do nothing
}





namespace VertexBufferMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create  = &metal_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete  = &metal_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update  = &metal_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map     = &metal_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap   = &metal_VertexBuffer_Unmap;
}

    void
SetToRHI( Handle vb, id<MTLRenderCommandEncoder> ce )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    [ce setVertexBuffer:self->uid offset:0 atIndex:0 ]; // CRAP: assuming vdata is buffer#0
     
}
}


} // namespace rhi
