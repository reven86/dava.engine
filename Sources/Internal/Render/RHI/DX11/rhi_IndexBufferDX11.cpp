

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx11.h"


namespace rhi
{
//==============================================================================


class
IndexBufferDX11_t
{
public:
                            IndexBufferDX11_t();
                            ~IndexBufferDX11_t();

    unsigned        _size;
    ID3D11Buffer*   _ib11;
    unsigned        _mapped:1;
};

typedef ResourcePool<IndexBufferDX11_t,RESOURCE_VERTEX_BUFFER>  IndexBufferDX11Pool;

RHI_IMPL_POOL(IndexBufferDX11_t,RESOURCE_VERTEX_BUFFER);


IndexBufferDX11_t::IndexBufferDX11_t()
  : _size(0),
    _ib11(nullptr),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

IndexBufferDX11_t::~IndexBufferDX11_t()
{
}


//==============================================================================


//------------------------------------------------------------------------------

static Handle
dx11_IndexBuffer_Create( const IndexBuffer::Descriptor& desc )
{
    Handle              handle = InvalidHandle;

    DVASSERT(desc.size);
    if( desc.size )
    {
        D3D11_BUFFER_DESC   desc11 = {0};
        ID3D11Buffer*       buf    = nullptr;
        DX11Command         cmd    = { DX11Command::CREATE_BUFFER, { uint64_t(&desc11), NULL, uint64_t(&buf) } };
        
        desc11.ByteWidth        = desc.size;        
        desc11.Usage            = D3D11_USAGE_DYNAMIC;
        desc11.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
        desc11.BindFlags        = D3D11_BIND_INDEX_BUFFER;                
        desc11.MiscFlags        = 0;
        
        ExecDX11( &cmd, 1 );

        if( SUCCEEDED(cmd.retval) )
        {
            handle = IndexBufferDX11Pool::Alloc();
            IndexBufferDX11_t*  ib = IndexBufferDX11Pool::Get( handle );

            ib->_size     = desc.size;
            ib->_ib11     = buf;
            ib->_mapped   = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D11ErrorText(cmd.retval) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void            
dx11_IndexBuffer_Delete( Handle vb )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( vb );

    if( self )
    {
        if( self->_ib11 )
        {
            DX11Command cmd[] = { DX11Command::RELEASE, { uint64_t(static_cast<IUnknown*>(self->_ib11)) } };

            ExecDX11( cmd, countof(cmd) );
            self->_ib11 = nullptr;
        }

        self->_size = 0;

        IndexBufferDX11Pool::Free( vb );
    }
}


//------------------------------------------------------------------------------
    
static bool
dx11_IndexBuffer_Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferDX11_t*  self    = IndexBufferDX11Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        D3D11_MAPPED_SUBRESOURCE    rc   = {0};
        DX11Command                 cmd1 = { DX11Command::MAP_RESOURCE, { uint64_t(self->_ib11), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };
        
        ExecDX11( &cmd1, 1 );

        if( rc.pData )
        {            
            DX11Command cmd2  = { DX11Command::UNMAP_RESOURCE, { uint64_t(self->_ib11), 0 } };

            memcpy( ((uint8*)(rc.pData))+offset, data, size );            
            
            ExecDX11( &cmd2, 1 );
            success = true;
        }
    }
    
    return success;
/*
    bool                success = false;
    IndexBufferDX11_t*  self    = IndexBufferDX11Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*       ptr  = nullptr;
        D3D11_BOX   dst;
        DX11Command cmd1 = { DX11Command::UPDATE_RESOURCE, { uint64_t(self->_ib11), 0, (uint64_t)(&dst), uint64(data), size, 0 } };
        
        dst.left   = offset;
        dst.right  = size;
        dst.top    = 0;
        dst.bottom = 1;
        dst.front  = 0;
        dst.back   = 1;

        ExecDX11( &cmd1, 1 );

        if( SUCCEEDED(cmd1.retval) )
        {
            success = true;
        }
    }

    return success;
*/
}


//------------------------------------------------------------------------------

static void*
dx11_IndexBuffer_Map( Handle vb, unsigned offset, unsigned size )
{
    void*                       ptr  = nullptr;
    IndexBufferDX11_t*          self = IndexBufferDX11Pool::Get( vb );
    D3D11_MAPPED_SUBRESOURCE    rc   = {0};
    DX11Command                 cmd  = { DX11Command::MAP_RESOURCE, { uint64_t(self->_ib11), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };

    DVASSERT(!self->_mapped);
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        ptr           = rc.pData;
        self->_mapped = true;
    }

    return ptr;
}


//------------------------------------------------------------------------------

static void
dx11_IndexBuffer_Unmap( Handle vb )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( vb );
    DX11Command         cmd  = { DX11Command::UNMAP_RESOURCE, { uint64_t(self->_ib11), 0 } };
    
    DVASSERT(self->_mapped);
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->_mapped = false;
    }
}



//------------------------------------------------------------------------------

namespace IndexBufferDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create   = &dx11_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete   = &dx11_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update   = &dx11_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map      = &dx11_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap    = &dx11_IndexBuffer_Unmap;
}


void 
SetToRHI( Handle ibh, unsigned offset )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ibh );    
    
    DVASSERT(!self->_mapped);
    _D3D11_ImmediateContext->IASetIndexBuffer( self->_ib11, DXGI_FORMAT_R16_UINT, offset );
}

}


} // namespace rhi
