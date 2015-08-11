

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
    Handle  handle = InvalidHandle;

    DVASSERT(desc.size);
    if( desc.size )
    {
        D3D11_BUFFER_DESC   desc11 = {0};
        ID3D11Buffer*       buf    = nullptr;
        
        desc11.ByteWidth        = desc.size;        
        desc11.Usage            = D3D11_USAGE_DYNAMIC;
        desc11.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
        desc11.BindFlags        = D3D11_BIND_INDEX_BUFFER;                
        desc11.MiscFlags        = 0;
        
        HRESULT hr = _D3D11_Device->CreateBuffer( &desc11, NULL, &buf );

        if( SUCCEEDED(hr) )
        {
            handle = IndexBufferDX11Pool::Alloc();
            IndexBufferDX11_t*    vb = IndexBufferDX11Pool::Get( handle );

            vb->_size   = desc.size;
            vb->_ib11   = buf;
            vb->_mapped = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D11ErrorText(hr) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void            
dx11_IndexBuffer_Delete( Handle ib )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ib );

    if( self )
    {
        if( self->_ib11 )
        {
            self->_ib11->Release();
            self->_ib11 = nullptr;
        }

        self->_size = 0;

        IndexBufferDX11Pool::Free( ib );
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
        D3D11_MAPPED_SUBRESOURCE    rc = {0};
        
        _D3D11_ImmediateContext->Map( self->_ib11, 0, D3D11_MAP_WRITE_DISCARD, 0, &rc );
        
        if( rc.pData )
        {            
            memcpy( ((uint8*)(rc.pData))+offset, data, size );            
            _D3D11_ImmediateContext->Unmap( self->_ib11, 0 );
            success = true;
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------

static void*
dx11_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    void*                       ptr  = nullptr;
    IndexBufferDX11_t*          self = IndexBufferDX11Pool::Get( ib );
    D3D11_MAPPED_SUBRESOURCE    rc   = {0};

    DVASSERT(!self->_mapped);
    _D3D11_ImmediateContext->Map( self->_ib11, 0, D3D11_MAP_WRITE_DISCARD, 0, &rc );

    if( rc.pData )
    {
        ptr           = rc.pData;
        self->_mapped = true;
    }

    return ((uint8*)ptr)+offset;
}


//------------------------------------------------------------------------------

static void
dx11_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ib );
    
    DVASSERT(self->_mapped);
    _D3D11_ImmediateContext->Unmap( self->_ib11, 0 );
    self->_mapped = false;
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
SetToRHI( Handle ibh, unsigned offset, ID3D11DeviceContext* context )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ibh );    
    
    DVASSERT(!self->_mapped);
    context->IASetIndexBuffer( self->_ib11, DXGI_FORMAT_R16_UINT, offset );
}

}


} // namespace rhi
