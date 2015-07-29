
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/format_convert.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx11.h"


namespace rhi
{
//==============================================================================

class
TextureDX11_t
{
public:

                                TextureDX11_t();


    TextureFormat               format;
    unsigned                    width;
    unsigned                    height;

    ID3D11Texture2D*            tex2d;
    ID3D11ShaderResourceView*   tex2d_srv;

    unsigned                    isMapped:1;
};


TextureDX11_t::TextureDX11_t()
  : width(0),
    height(0),
    tex2d(nullptr),
    tex2d_srv(nullptr),
    isMapped(false)
{
}

typedef ResourcePool<TextureDX11_t,RESOURCE_TEXTURE> TextureDX11Pool;
RHI_IMPL_POOL(TextureDX11_t,RESOURCE_TEXTURE);

//------------------------------------------------------------------------------

static Handle
dx11_Texture_Create( const Texture::Descriptor& desc )
{
    DVASSERT(desc.levelCount);

    Handle                  handle = InvalidHandle;
    D3D11_TEXTURE2D_DESC    desc2d = {0};
    ID3D11Texture2D*        tex2d  = nullptr;

    desc2d.Width                = desc.width;
    desc2d.Height               = desc.height;
    desc2d.MipLevels            = desc.levelCount;
    desc2d.ArraySize            = 1;
    desc2d.Format               = DX11_TextureFormat( desc.format );
    desc2d.SampleDesc.Count     = 1;
    desc2d.SampleDesc.Quality   = 0;
    desc2d.Usage                = D3D11_USAGE_DYNAMIC;
    desc2d.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    desc2d.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    desc2d.MiscFlags            = 0;
    
    if( desc.autoGenMipmaps )
        desc2d.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

    DX11Command cmd1 = { DX11Command::CREATE_TEXTURE2D, { (uint64)(&desc2d), 0, (uint64)(&tex2d) } };

    ExecDX11( &cmd1, 1 );
    if( SUCCEEDED(cmd1.retval) )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC view;
        ID3D11ShaderResourceView*       srv = nullptr;

        view.Format                     = desc2d.Format;
        view.ViewDimension              = D3D11_SRV_DIMENSION_TEXTURE2D;
        view.Texture2D.MipLevels        = desc2d.MipLevels;
        view.Texture2D.MostDetailedMip  = 0;

        DX11Command cmd2 = { DX11Command::CREATE_SHADEER_RESOURCE_VIEW, { (uint64)(tex2d), (uint64)(&view), (uint64)(&srv) } };

        ExecDX11( &cmd2, 1 );
        if( SUCCEEDED(cmd2.retval) )
        {
            handle  = TextureDX11Pool::Alloc();
            TextureDX11_t*  tex = TextureDX11Pool::Get( handle );

            tex->tex2d      = tex2d;
            tex->tex2d_srv  = srv;
            tex->isMapped   = false;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx11_Texture_Delete( Handle tex )
{
    if( tex != InvalidHandle )
    {
        TextureDX11_t*  self = TextureDX11Pool::Get( tex );
        DX11Command     cmd[] = 
        {
            { DX11Command::RELEASE, { uint64_t(static_cast<IUnknown*>(self->tex2d)) } }
        };

        ExecDX11( cmd, countof(cmd) );


        TextureDX11Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

static void*
dx11_Texture_Map( Handle tex, unsigned level, TextureFace face )
{
    TextureDX11_t*              self = TextureDX11Pool::Get( tex );
    void*                       mem  = nullptr;
    D3D11_MAPPED_SUBRESOURCE    rc   = {0};
    DX11Command                 cmd  = { DX11Command::MAP_RESOURCE, { uint64_t(self->tex2d), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };

    DVASSERT(!self->isMapped);
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        mem            = rc.pData;
        self->isMapped = true;
    }

    return mem;
}


//------------------------------------------------------------------------------

static void
dx11_Texture_Unmap( Handle tex )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );
    DX11Command     cmd  = { DX11Command::UNMAP_RESOURCE, { uint64_t(self->tex2d), 0 } };
    
    DVASSERT(self->isMapped);
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->isMapped = false;
    }
}


//------------------------------------------------------------------------------

void
dx11_Texture_Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );
    void*           dst  = dx11_Texture_Map( tex, level, face );
    uint32          sz   = TextureSize( self->format, self->width, self->height, level );
    
    memcpy( dst, data, sz );
    dx11_Texture_Unmap( tex );
}


//==============================================================================

namespace TextureDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Texture_Create = &dx11_Texture_Create;
    dispatch->impl_Texture_Delete = &dx11_Texture_Delete;
    dispatch->impl_Texture_Map    = &dx11_Texture_Map;
    dispatch->impl_Texture_Unmap  = &dx11_Texture_Unmap;
    dispatch->impl_Texture_Update = &dx11_Texture_Update;
}


void
SetToRHI( Handle tex, unsigned unit_i )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );
    
    _D3D11_ImmediateContext->PSSetShaderResources( unit_i, 1, &(self->tex2d_srv) );
}


void
SetAsRenderTarget( Handle tex )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );
    
}

void
SetAsDepthStencil( Handle tex )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );
}


}

} // namespace rhi

