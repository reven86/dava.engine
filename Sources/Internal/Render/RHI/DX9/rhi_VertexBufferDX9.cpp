

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
VertexBufferDX9_t
{
public:
                            VertexBufferDX9_t();
                            ~VertexBufferDX9_t();

    unsigned                _size;
    IDirect3DVertexBuffer9* _vb9;
    unsigned                _mapped:1;
};

typedef Pool<VertexBufferDX9_t,RESOURCE_VERTEX_BUFFER>   VertexBufferDX9Pool;

RHI_IMPL_POOL(VertexBufferDX9_t,RESOURCE_VERTEX_BUFFER);


VertexBufferDX9_t::VertexBufferDX9_t()
  : _size(0),
    _vb9(nullptr),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

VertexBufferDX9_t::~VertexBufferDX9_t()
{
}


//==============================================================================


//------------------------------------------------------------------------------

static Handle
dx9_VertexBuffer_Create( unsigned size, uint32 options )
{
    Handle  handle = InvalidHandle;

    DVASSERT(size);
    if( size )
    {
        IDirect3DVertexBuffer9* vb9  = nullptr;
        HRESULT                 hr   = _D3D9_Device->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vb9, NULL );

        if( SUCCEEDED(hr) )
        {
            handle = VertexBufferDX9Pool::Alloc();
            VertexBufferDX9_t*    vb = VertexBufferDX9Pool::Get( handle );

            vb->_size     = size;
            vb->_vb9      = vb9;
            vb->_mapped   = false;
        }
        else
        {
            Logger::Error( "FAILED to create vertex-buffer:\n%s\n", D3D9ErrorText(hr) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void            
dx9_VertexBuffer_Delete( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );

    if( self )
    {
        if( self->_vb9 )
        {
            self->_vb9->Release();
            self->_vb9 = nullptr;
         }

        self->_size = 0;
    }
}


//------------------------------------------------------------------------------
    
static bool
dx9_VertexBuffer_Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    VertexBufferDX9_t*  self    = VertexBufferDX9Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*   ptr = nullptr;
        HRESULT hr  = self->_vb9->Lock( offset, size, &ptr, 0 );

        if( SUCCEEDED(hr) )
        {
            memcpy( ptr, data, size );
            self->_vb9->Unlock();
            success = true;
        }
        else
        {
            Logger::Error( "FAILED to lock vertex-buffer:\n%s\n", D3D9ErrorText(hr) );
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
dx9_VertexBuffer_Map( Handle vb, unsigned offset, unsigned size )
{
    void*               ptr  = nullptr;
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    HRESULT             hr   = self->_vb9->Lock( offset, size, &ptr, 0 );

    DVASSERT(!self->_mapped);

    if( SUCCEEDED(hr) )
    {
        self->_mapped = true;
    }
    else
    {
        ptr = nullptr;
        Logger::Error( "FAILED to lock vertex-buffer:\n%s\n", D3D9ErrorText(hr) );
    }

    return ptr;
}


//------------------------------------------------------------------------------

static void
dx9_VertexBuffer_Unmap( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    HRESULT             hr   = self->_vb9->Unlock();

    DVASSERT(self->_mapped);

    if( SUCCEEDED(hr) )
    {
        self->_mapped = false;
    }
    else
    {
        Logger::Error( "FAILED to unlock vertex-buffer:\n%s\n", D3D9ErrorText(hr) );
    }
}



//------------------------------------------------------------------------------

namespace VertexBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create  = &dx9_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete  = &dx9_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update  = &dx9_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map     = &dx9_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap   = &dx9_VertexBuffer_Unmap;
}


void 
SetToRHI( Handle vb, unsigned stream_i, unsigned offset, unsigned stride  )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    HRESULT             hr   = _D3D9_Device->SetStreamSource( stream_i, self->_vb9, offset, stride );

    DVASSERT(!self->_mapped);

    if( FAILED(hr) )    
        Logger::Error( "SetStreamSource failed:\n%s\n", D3D9ErrorText(hr) );
}

}


} // namespace rhi
