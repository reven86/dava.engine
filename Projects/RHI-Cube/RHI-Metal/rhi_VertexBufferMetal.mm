

    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
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

typedef Pool<VertexBufferMetal_t>   VertexBufferMetalPool;


//==============================================================================


namespace VertexBuffer
{
//------------------------------------------------------------------------------

Handle
Create( uint32 size, uint32 options )
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

void            
Delete( Handle vb )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    if( self )
    {

        delete self;
    }
}


//------------------------------------------------------------------------------
    
bool
Update( Handle vb, const void* data, uint32 offset, uint32 size )
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

void*
Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    DVASSERT(self->data);
    
    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

void
Unmap( Handle vb )
{
    // do nothing
}


} // namespace VertexBuffer





namespace VertexBufferMetal
{
void
SetToRHI( Handle vb, id<MTLRenderCommandEncoder> ce )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    [ce setVertexBuffer:self->uid offset:0 atIndex:0 ]; // CRAP: assuming vdata is buffer#0
     
}
}


} // namespace rhi
