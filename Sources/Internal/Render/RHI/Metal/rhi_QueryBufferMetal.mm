
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

class
QueryBufferMetal_t
{
public:
                    QueryBufferMetal_t();
                    ~QueryBufferMetal_t();

    uint32          maxObjectCount;
    id<MTLBuffer>   uid;
};

typedef Pool<QueryBufferMetal_t,RESOURCE_QUERY_BUFFER>  QueryBufferMetalPool;
RHI_IMPL_POOL(QueryBufferMetal_t,RESOURCE_QUERY_BUFFER);


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
metal_QueryBuffer_Create( uint32 maxObjectCount )
{
    Handle              handle = QueryBufferMetalPool::Alloc();
    QueryBufferMetal_t* buf    = QueryBufferMetalPool::Get( handle );

    if( buf )
    {
        int             sz  = maxObjectCount * QueryBUfferElemeentAlign;
        id<MTLBuffer>   uid = [_Metal_Device newBufferWithLength:sz options:MTLResourceOptionCPUCacheModeDefault];
    
        buf->uid            = uid;
        buf->maxObjectCount = maxObjectCount;
        
        memset( [uid contents], 0x00, sz );
    }

    return handle;
}

static void
metal_QueryBuffer_Delete( Handle handle )
{
    QueryBufferMetal_t* buf = QueryBufferMetalPool::Get( handle );

    if( buf )
    {
        [buf->uid release];
        buf->uid = nil;
    }

    QueryBufferMetalPool::Free( handle );
}

static bool
metal_QueryBuffer_IsReady( Handle handle, uint32 objectIndex )
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
metal_QueryBuffer_Value( Handle handle, uint32 objectIndex )
{
    int                 value = 0;
    QueryBufferMetal_t* buf   = QueryBufferMetalPool::Get( handle );

    if( buf  &&  objectIndex < buf->maxObjectCount )
    {
        uint8* data = (uint8*)([buf->uid contents]) + objectIndex*QueryBUfferElemeentAlign;
    
        value = *((uint32*)data);
    }

    return value;
}


namespace QueryBufferMetal
{

id<MTLBuffer> GetBuffer( Handle handle )
{
    QueryBufferMetal_t* buf   = QueryBufferMetalPool::Get( handle );
    
    return (buf)  ? buf->uid  : nil;
}

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_QueryBuffer_Create   = &metal_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Delete   = &metal_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady  = &metal_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value    = &metal_QueryBuffer_Value;
}

}


//==============================================================================
} // namespace rhi

