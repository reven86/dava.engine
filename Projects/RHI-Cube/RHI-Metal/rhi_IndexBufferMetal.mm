
    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
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

namespace IndexBuffer
{
//------------------------------------------------------------------------------

Handle
Create( uint32 size, uint32 options )
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

void
Delete( Handle ib )
{
    IndexBufferMetal_t*    self = IndexBufferMetalPool::Get( ib );

    if( self )
    {
        delete self;
    }
}


//------------------------------------------------------------------------------
    
bool
Update( Handle ib, const void* data, unsigned offset, unsigned size )
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

void*
Map( Handle ib, unsigned offset, unsigned size )
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get( ib );

    DVASSERT(self->data);
    
    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

void
Unmap( Handle ib )
{
    // do nothing
}

} // namespace IndexBuffer


//------------------------------------------------------------------------------

namespace IndexBufferMetal
{

id<MTLBuffer> 
GetBuffer( Handle ib )
{
    IndexBufferMetal_t* self = IndexBufferMetalPool::Get( ib );

    return (self)  ? self->uid  : nil;
}


} // namespace IndexBufferGLES

//==============================================================================
} // namespace rhi

