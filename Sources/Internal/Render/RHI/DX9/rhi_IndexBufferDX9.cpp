
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
  : _ib9(nullptr),
    _size(0),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

IndexBufferDX9_t::~IndexBufferDX9_t()
{
}


//------------------------------------------------------------------------------

static Handle
dx9_IndexBuffer_Create( unsigned size, uint32 options )
{
    Handle  handle = InvalidIndex;

    DVASSERT(size);
    if( size )
    {
        IDirect3DIndexBuffer9*  ib9   = nullptr;
        DX9Command              cmd[] = { { DX9Command::CREATE_INDEX_BUFFER, { size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, uint64_t(&ib9), NULL } } };
        
        ExecDX9( cmd, countof(cmd) );

        if( SUCCEEDED(cmd[0].retval) )
        {
            handle = IndexBufferDX9Pool::Alloc();

            IndexBufferDX9_t* ib = IndexBufferDX9Pool::Get( handle );
            
            ib->_size   = size;
            ib->_ib9    = ib9;
            ib->_mapped = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D9ErrorText(cmd[0].retval) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_IndexBuffer_Delete( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    
    if( self )
    {
        if( self->_ib9 )
        {
            DX9Command  cmd[] = { DX9Command::RELEASE, { uint64_t(static_cast<IUnknown*>(self->_ib9)) } };

            ExecDX9( cmd, countof(cmd) );
            self->_ib9 = nullptr;
        }

        self->_size = 0;
    }

}


//------------------------------------------------------------------------------
    
static bool
dx9_IndexBuffer_Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferDX9_t*   self    = IndexBufferDX9Pool::Get( ib );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*       ptr  = nullptr;
        DX9Command  cmd1 = { DX9Command::LOCK_INDEX_BUFFER, { uint64_t(self->_ib9), offset, size, uint64_t(&ptr), 0 } };
        
        ExecDX9( &cmd1, 1 );
        if( SUCCEEDED(cmd1.retval) )
        {
            memcpy( ptr, data, size );

            DX9Command  cmd2 = { DX9Command::UNLOCK_INDEX_BUFFER, { uint64_t(self->_ib9) } };
            
            ExecDX9( &cmd2, 1 );
            success = true;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
dx9_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    void*               ptr  = nullptr;
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    DX9Command          cmd  = { DX9Command::LOCK_INDEX_BUFFER, { uint64_t(self->_ib9), offset, size, uint64_t(&ptr), 0 } };

    DVASSERT(!self->_mapped);
    ExecDX9( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->_mapped = true;
    }

    return ptr;
}


//------------------------------------------------------------------------------

static void
dx9_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    DX9Command          cmd  = { DX9Command::UNLOCK_INDEX_BUFFER, { uint64_t(self->_ib9) } };
    
    DVASSERT(self->_mapped);
    ExecDX9( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->_mapped = false;
    }
}



//------------------------------------------------------------------------------

namespace IndexBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create   = &dx9_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete   = &dx9_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update   = &dx9_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map      = &dx9_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap    = &dx9_IndexBuffer_Unmap;
}

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

