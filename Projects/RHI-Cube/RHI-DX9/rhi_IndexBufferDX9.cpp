
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
IndexBufferDX9_t
{
public:
                            IndexBufferDX9_t();
                            ~IndexBufferDX9_t();


    IDirect3DIndexBuffer9*  _ib9;
    unsigned                _size;
    unsigned                _mapped:1;
};

typedef Pool<IndexBufferDX9_t,RESOURCE_INDEX_BUFFER>    IndexBufferDX9Pool;
RHI_IMPL_POOL(IndexBufferDX9_t,RESOURCE_INDEX_BUFFER);


//==============================================================================


IndexBufferDX9_t::IndexBufferDX9_t()
  : _size(0),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

IndexBufferDX9_t::~IndexBufferDX9_t()
{
}


namespace IndexBuffer
{
//------------------------------------------------------------------------------

Handle
Create( unsigned size, uint32 options )
{
    Handle  handle = InvalidIndex;

    DVASSERT(size);
    if( size )
    {
        IDirect3DIndexBuffer9*  ib9 = 0;
        HRESULT                 hr  = _D3D9_Device->CreateIndexBuffer( size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ib9, NULL );

        if( SUCCEEDED(hr) )
        {
            handle = IndexBufferDX9Pool::Alloc();

            IndexBufferDX9_t* ib = IndexBufferDX9Pool::Get( handle );
            
            ib->_size   = size;
            ib->_ib9    = ib9;
            ib->_mapped = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D9ErrorText(hr) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void
Delete( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    
    if( self )
    {
        if( self->_ib9 )
        {
            self->_ib9->Release();
            self->_ib9 = 0;
        }

        self->_size = 0;
        
        delete self;
    }

}


//------------------------------------------------------------------------------
    
bool
Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferDX9_t*   self    = IndexBufferDX9Pool::Get( ib );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*   ptr = 0;
        HRESULT hr  = self->_ib9->Lock( offset, size, &ptr, 0 );

        if( SUCCEEDED(hr) )
        {
            memcpy( ptr, data, size );
            self->_ib9->Unlock();

            success = true;
        }
        else
        {
            Logger::Error( "FAILED to lock index-buffer:\n%s\n", D3D9ErrorText(hr) );
        }
    }

    return success;
}


//------------------------------------------------------------------------------

void*
Map( Handle ib, unsigned offset, unsigned size )
{
    void*               ptr  = 0;
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    HRESULT             hr  = self->_ib9->Lock( offset, size, &ptr, 0 );

    DVASSERT(!self->_mapped);

    if( SUCCEEDED(hr) )
    {
        self->_mapped = true;
    }
    else
    {
        ptr = 0;
        Logger::Error( "FAILED to lock index-buffer:\n%s\n", D3D9ErrorText(hr) );
    }

    return ptr;
}


//------------------------------------------------------------------------------

void
Unmap( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    HRESULT             hr   = self->_ib9->Unlock();

    DVASSERT(self->_mapped);

    if( SUCCEEDED(hr) )
    {
        self->_mapped = false;
    }
    else
    {
        Logger::Error( "FAILED to unlock index-buffer:\n%s\n", D3D9ErrorText(hr) );
    }
}


} // namespace IndexBuffer





//------------------------------------------------------------------------------

namespace IndexBufferDX9
{

void 
SetToRHI( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    HRESULT             hr   = _D3D9_Device->SetIndices( self->_ib9 );

    DVASSERT(!self->_mapped);

    if( FAILED(hr) )    
        Logger::Error( "SetIndices failed:\n%s\n", D3D9ErrorText(hr) );
}

}

//==============================================================================
} // namespace rhi

