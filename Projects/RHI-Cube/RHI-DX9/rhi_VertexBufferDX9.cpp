

    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
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

typedef Pool<VertexBufferDX9_t>   VertexBufferDX9Pool;

RHI_IMPL_POOL(VertexBufferDX9_t);


VertexBufferDX9_t::VertexBufferDX9_t()
  : _size(0),
    _vb9(0),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

VertexBufferDX9_t::~VertexBufferDX9_t()
{
}


//==============================================================================


namespace VertexBuffer
{

//------------------------------------------------------------------------------

Handle
Create( unsigned size, uint32 options )
{
    Handle  handle = InvalidHandle;

    DVASSERT(size);
    if( size )
    {
        IDirect3DVertexBuffer9* vb9  = 0;
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

void            
Delete( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );

    if( self )
    {
        if( self->_vb9 )
        {
            self->_vb9->Release();
            self->_vb9 = 0;
         }

        self->_size = 0;
        
        delete self;
    }
}


//------------------------------------------------------------------------------
    
bool
Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    VertexBufferDX9_t*  self    = VertexBufferDX9Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*   ptr = 0;
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

void*
Map( Handle vb, unsigned offset, unsigned size )
{
    void*               ptr  = 0;
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    HRESULT             hr   = self->_vb9->Lock( offset, size, &ptr, 0 );

    DVASSERT(!self->_mapped);

    if( SUCCEEDED(hr) )
    {
        self->_mapped = true;
    }
    else
    {
        ptr = 0;
        Logger::Error( "FAILED to lock vertex-buffer:\n%s\n", D3D9ErrorText(hr) );
    }

    return ptr;
}


//------------------------------------------------------------------------------

void
Unmap( Handle vb )
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

} // namespace VertexBuffer



//------------------------------------------------------------------------------

namespace VertexBufferDX9
{

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
