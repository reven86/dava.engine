
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

class
QueryBufferDX9_t
{
public:
                                    QueryBufferDX9_t();
                                    ~QueryBufferDX9_t();

    std::vector<IDirect3DQuery9*>   query;
};

typedef Pool<QueryBufferDX9_t,RESOURCE_QUERY_BUFFER>    QueryBufferDX9Pool;
RHI_IMPL_POOL(QueryBufferDX9_t,RESOURCE_QUERY_BUFFER);


//==============================================================================


QueryBufferDX9_t::QueryBufferDX9_t()
{
}


//------------------------------------------------------------------------------

QueryBufferDX9_t::~QueryBufferDX9_t()
{
}

static Handle
dx9_QueryBuffer_Create( uint32 maxObjectCount )
{
    Handle              handle = QueryBufferDX9Pool::Alloc();
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf )
    {
        buf->query.resize( maxObjectCount );
        memset( &(buf->query[0]), 0, sizeof(buf->query[0])*buf->query.size() );
    }

    return handle;
}

static void
dx9_QueryBuffer_Delete( Handle handle )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf )
    {
        for( std::vector<IDirect3DQuery9*>::iterator q=buf->query.begin(),q_end=buf->query.end(); q!=q_end; ++q )
        {
            IDirect3DQuery9*    iq = *q;

            if( iq )
                iq->Release();
        }

        buf->query.clear();
    }

    QueryBufferDX9Pool::Free( handle );
}

static bool
dx9_QueryBuffer_IsReady( Handle handle, uint32 objectIndex )
{
    bool                ready = false;
    QueryBufferDX9_t*   buf   = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        if( iq )
        {
            DWORD       val;
            HRESULT     hr  = iq->GetData( &val, sizeof(val), 0 ); // DO NOT flush

            if( SUCCEEDED(hr) )
            {
                ready = hr == S_OK;
            }
        }
    }

    return ready;
}

static int
dx9_QueryBuffer_Value( Handle handle, uint32 objectIndex )
{
    int                 value = 0;
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        if( iq )
        {
            DWORD       val = 0;
            HRESULT     hr  = iq->GetData( &val, sizeof(val), 0 ); // DO NOT flush

            if( hr == S_OK )
            {
                value = val;
            }
        }
    }

    return value;
}


namespace QueryBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_QueryBuffer_Create   = &dx9_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Delete   = &dx9_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady  = &dx9_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value    = &dx9_QueryBuffer_Value;
}

void
BeginQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];

        if( !iq )
        {
            HRESULT hr = _D3D9_Device->CreateQuery( D3DQUERYTYPE_OCCLUSION, &iq );
            
            if( SUCCEEDED(hr) )
            {
                buf->query[objectIndex] = iq;
            }
            else
            {
                iq = nullptr;
            }
        }

        if( iq )
        {
            iq->Issue( D3DISSUE_BEGIN );
        }
    }
}


void
EndQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        DVASSERT(iq);
        iq->Issue( D3DISSUE_END );
    }
}

}


//==============================================================================
} // namespace rhi

