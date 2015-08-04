
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

    void*                       mappedData;
    unsigned                    mappedLevel;

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
    HRESULT                 hr;

    desc2d.Width                = desc.width;
    desc2d.Height               = desc.height;
    desc2d.MipLevels            = desc.levelCount;
    desc2d.ArraySize            = 1;
    desc2d.Format               = DX11_TextureFormat( desc.format );
    desc2d.SampleDesc.Count     = 1;
    desc2d.SampleDesc.Quality   = 0;
    desc2d.Usage                = D3D11_USAGE_DEFAULT;
    desc2d.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    desc2d.CPUAccessFlags       = 0;//D3D11_CPU_ACCESS_WRITE;
    desc2d.MiscFlags            = 0;

    DVASSERT(desc2d.Format!=DXGI_FORMAT_UNKNOWN);
    
    if( desc.autoGenMipmaps )
        desc2d.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

    hr = _D3D11_Device->CreateTexture2D( &desc2d, NULL, &tex2d );

    if( SUCCEEDED(hr) )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC view;
        ID3D11ShaderResourceView*       srv = nullptr;

        view.Format                     = desc2d.Format;
        view.ViewDimension              = D3D11_SRV_DIMENSION_TEXTURE2D;
        view.Texture2D.MipLevels        = desc2d.MipLevels;
        view.Texture2D.MostDetailedMip  = 0;

        hr = _D3D11_Device->CreateShaderResourceView( tex2d, &view, &srv );

        if( SUCCEEDED(hr) )
        {
            handle  = TextureDX11Pool::Alloc();
            TextureDX11_t*  tex = TextureDX11Pool::Get( handle );

            tex->tex2d      = tex2d;
            tex->tex2d_srv  = srv;
            tex->format     = desc.format;
            tex->width      = desc.width;
            tex->height     = desc.height;
            tex->mappedData = nullptr;
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
        
        self->tex2d_srv->Release();
        self->tex2d_srv = nullptr;
        
        self->tex2d->Release();
        self->tex2d = nullptr;

        if( self->mappedData )
        {
            ::free( self->mappedData );
            self->mappedData = nullptr;
        }

        TextureDX11Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

static void*
dx11_Texture_Map( Handle tex, unsigned level, TextureFace face )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );

    DVASSERT(!self->isMapped);
    self->mappedData  = ::realloc( self->mappedData, TextureSize(self->format,self->width,self->height,level) );
    self->mappedLevel = level;
    self->isMapped    = true;

    if( self->format == TEXTURE_FORMAT_R8G8B8A8 )
    {
        _SwapRB8( self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel) );
    }
    else if( self->format == TEXTURE_FORMAT_R4G4B4A4 )
    {
        _SwapRB4( self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel) );
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _SwapRB5551( self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel) );
    }

    return self->mappedData;
}


//------------------------------------------------------------------------------

static void
dx11_Texture_Unmap( Handle tex )
{
    TextureDX11_t*  self = TextureDX11Pool::Get( tex );

    DVASSERT(self->isMapped);

    if (self->format == TEXTURE_FORMAT_R8G8B8A8)
    {
        _SwapRB8(self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }
    else if (self->format == TEXTURE_FORMAT_R4G4B4A4)
    {
        _SwapRB4(self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _SwapRB5551(self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }

    _D3D11_ImmediateContext->UpdateSubresource
    (
        self->tex2d,
        self->mappedLevel,
        NULL,
        self->mappedData,
        TextureStride(self->format,Size2i(self->width,self->height),self->mappedLevel),
        0
    );
    self->isMapped = false;
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

