

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
VertexBufferDX11_t
{
public:
                            VertexBufferDX11_t();
                            ~VertexBufferDX11_t();

    unsigned        _size;
    ID3D11Buffer*   _vb11;
    unsigned        _mapped:1;
};

typedef ResourcePool<VertexBufferDX11_t,RESOURCE_VERTEX_BUFFER>   VertexBufferDX11Pool;

RHI_IMPL_POOL(VertexBufferDX11_t,RESOURCE_VERTEX_BUFFER);


VertexBufferDX11_t::VertexBufferDX11_t()
  : _size(0),
    _vb11(nullptr),
    _mapped(false)
{
}


//------------------------------------------------------------------------------

VertexBufferDX11_t::~VertexBufferDX11_t()
{
}


//==============================================================================


//------------------------------------------------------------------------------

static Handle
dx11_VertexBuffer_Create( const VertexBuffer::Descriptor& desc )
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
        desc11.BindFlags        = D3D11_BIND_VERTEX_BUFFER;                
        desc11.MiscFlags        = 0;
        
        ExecDX11( &cmd, 1 );

        if( SUCCEEDED(cmd.retval) )
        {
            handle = VertexBufferDX11Pool::Alloc();
            VertexBufferDX11_t*    vb = VertexBufferDX11Pool::Get( handle );

            vb->_size     = desc.size;
            vb->_vb11     = buf;
            vb->_mapped   = false;
        }
        else
        {
            Logger::Error( "FAILED to create vertex-buffer:\n%s\n", D3D11ErrorText(cmd.retval) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void            
dx11_VertexBuffer_Delete( Handle vb )
{
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get( vb );

    if( self )
    {
        if( self->_vb11 )
        {
            DX11Command cmd[] = { DX11Command::RELEASE, { uint64_t(static_cast<IUnknown*>(self->_vb11)) } };

            ExecDX11( cmd, countof(cmd) );
            self->_vb11 = nullptr;
        }

        self->_size = 0;

        VertexBufferDX11Pool::Free( vb );
    }
}


//------------------------------------------------------------------------------
    
static bool
dx11_VertexBuffer_Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    VertexBufferDX11_t* self    = VertexBufferDX11Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        D3D11_MAPPED_SUBRESOURCE    rc   = {0};
        DX11Command                 cmd1 = { DX11Command::MAP_RESOURCE, { uint64_t(self->_vb11), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };
        
        ExecDX11( &cmd1, 1 );

        if( rc.pData )
        {            
            DX11Command cmd2  = { DX11Command::UNMAP_RESOURCE, { uint64_t(self->_vb11), 0 } };

            memcpy( ((uint8*)(rc.pData))+offset, data, size );            
            
            ExecDX11( &cmd2, 1 );
            success = true;
        }
    }
    
    return success;
/*
    bool                success = false;
    VertexBufferDX11_t* self    = VertexBufferDX11Pool::Get( vb );

    DVASSERT(!self->_mapped);

    if( offset+size <= self->_size )
    {
        void*       ptr  = nullptr;
        D3D11_BOX   dst;
        DX11Command cmd1 = { DX11Command::UPDATE_RESOURCE, { uint64_t(self->_vb11), 0, (uint64_t)(&dst), uint64(data), size, 0 } };
        
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
dx11_VertexBuffer_Map( Handle vb, unsigned offset, unsigned size )
{
    void*                       ptr  = nullptr;
    VertexBufferDX11_t*         self = VertexBufferDX11Pool::Get( vb );
    D3D11_MAPPED_SUBRESOURCE    rc   = {0};
    DX11Command                 cmd  = { DX11Command::MAP_RESOURCE, { uint64_t(self->_vb11), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };

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
dx11_VertexBuffer_Unmap( Handle vb )
{
    VertexBufferDX11_t* self = VertexBufferDX11Pool::Get( vb );
    DX11Command         cmd  = { DX11Command::UNMAP_RESOURCE, { uint64_t(self->_vb11), 0 } };
    
    DVASSERT(self->_mapped);
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->_mapped = false;
    }
}



//------------------------------------------------------------------------------

namespace VertexBufferDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create  = &dx11_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete  = &dx11_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update  = &dx11_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map     = &dx11_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap   = &dx11_VertexBuffer_Unmap;
}


void 
SetToRHI( Handle vbh, unsigned stream_i, unsigned offset, unsigned stride  )
{
    VertexBufferDX11_t* self         = VertexBufferDX11Pool::Get( vbh );
    ID3D11Buffer*       vb[1]        = { self->_vb11 };
    UINT                vb_offset[1] = { offset };
    UINT                vb_stride[1] = { stride };
    
    
    DVASSERT(!self->_mapped);
    _D3D11_ImmediateContext->IASetVertexBuffers( stream_i, 1, vb, vb_stride, vb_offset );
}

}


} // namespace rhi
