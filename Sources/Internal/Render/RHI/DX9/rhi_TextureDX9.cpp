
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
TextureDX9_t
{
public:

                                TextureDX9_t();


    TextureFormat               format;
    unsigned                    width;
    unsigned                    height;

    IDirect3DBaseTexture9*      basetex9;
    IDirect3DTexture9*          tex9;    
    IDirect3DCubeTexture9*      cubetex9;
    mutable IDirect3DSurface9*  surf9;

    unsigned                    lastUnit;
    unsigned                    mappedLevel;
    TextureFace                 mappedFace;
    void*                       mappedData;
    unsigned                    isMapped:1;
};


TextureDX9_t::TextureDX9_t()
  : width(0),
    height(0),
    basetex9(nullptr),
    tex9(nullptr), 
    cubetex9(nullptr),
    surf9(nullptr),
    lastUnit(InvalidIndex),
    mappedData(nullptr),
    isMapped(false)
{
}

typedef Pool<TextureDX9_t,RESOURCE_TEXTURE>   TextureDX9Pool;
RHI_IMPL_POOL(TextureDX9_t,RESOURCE_TEXTURE);


//------------------------------------------------------------------------------

static void
_SwapRB8( void* data, uint32 size )
{
    for( uint8* d=(uint8*)data,*d_end=(uint8*)data+size; d!=d_end; d+=4 )
    {
        uint8   t = d[0];

        d[0] = d[2];
        d[2] = t;
    }
}


//------------------------------------------------------------------------------

static void
_SwapRB4( void* data, uint32 size )
{
    for( uint8* d=(uint8*)data,*d_end=(uint8*)data+size; d!=d_end; d+=2 )
    {
        uint8   t0 = d[0];
        uint8   t1 = d[1];

        d[0] = (t0&0x00FF) | (t1&0x00FF);
        d[1] = (t1&0x00FF) | (t0&0x00FF);
    }
}


//------------------------------------------------------------------------------

static Handle
dx9_Texture_Create( const Texture::Descriptor& desc )
{
    Handle              handle      = InvalidHandle;
    TextureDX9_t*       tex         = nullptr;
    IDirect3DTexture9*  tex9        = nullptr;
    DWORD               usage       =(desc.isRenderTarget)  ? D3DUSAGE_RENDERTARGET  : 0;
    D3DPOOL             pool        = (desc.isRenderTarget/*  ||  options&TEXTURE_OPT_DYNAMIC*/)  ? D3DPOOL_DEFAULT  : D3DPOOL_MANAGED;
    HRESULT             hr          = E_FAIL;
    bool                auto_mip    = (desc.autoGenMipmaps)  ? true  : false;
    unsigned            mip_count   = 0;


//    if( options&TEXTURE_OPT_DYNAMIC )
//        usage |= D3DUSAGE_DYNAMIC;

    if( desc.isRenderTarget )
        mip_count = 1;

    
    switch( desc.type )
    {
        case TEXTURE_TYPE_2D :
        {
            hr = _D3D9_Device->CreateTexture( desc.width, desc.height,
                                              mip_count,
                                              usage,
                                              DX9_TextureFormat(desc.format),
                                              pool,
                                              &tex9,
                                              NULL
                                            );
            if( SUCCEEDED(hr) )
            {
                handle = TextureDX9Pool::Alloc();
                tex    = TextureDX9Pool::Get( handle );

                tex->tex9 = tex9;

                hr = tex9->QueryInterface( IID_IDirect3DBaseTexture9, (void**)(&tex->basetex9) );
                if( tex->basetex9  &&  auto_mip )
                    tex->basetex9->SetAutoGenFilterType( D3DTEXF_LINEAR );

                if( desc.isRenderTarget/*  ||  options&TEXTURE_OPT_DYNAMIC*/ )
                {
                    hr = tex9->GetSurfaceLevel( 0, &tex->surf9 );
                }

                tex->width  = desc.width;
                tex->height = desc.height;
                tex->format = desc.format;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }

        }   break;
        
        case TEXTURE_TYPE_CUBE :
        {
            IDirect3DCubeTexture9*  cubetex9 = nullptr;
            
            hr = _D3D9_Device->CreateCubeTexture( desc.width, mip_count, usage, DX9_TextureFormat(desc.format), pool, &cubetex9, NULL );
        
            if( SUCCEEDED(hr) )
            {
                handle = TextureDX9Pool::Alloc();
                tex    = TextureDX9Pool::Get( handle );

                tex->cubetex9 = cubetex9;
                tex->tex9     = nullptr;

                hr = cubetex9->QueryInterface( IID_IDirect3DBaseTexture9, (void**)(&tex->basetex9) );
                if( tex->basetex9  &&  auto_mip )
                    tex->basetex9->SetAutoGenFilterType( D3DTEXF_LINEAR );
                
                tex->width  = desc.width;
                tex->height = desc.height;
                tex->format = desc.format;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }
        }   break;

    }
    
    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_Texture_Delete( Handle tex )
{
    if( tex != InvalidHandle )
    {
        TextureDX9_t* self = TextureDX9Pool::Get( tex );

        DVASSERT(!self->isMapped);

        if( self->surf9 )
        {
            self->surf9->Release();
            self->surf9 = nullptr;
        }

        if( self->basetex9 )
        {
            self->basetex9->Release();
            self->basetex9 = nullptr;
        }

        if( self->tex9 )
        {
            self->tex9->Release();
            self->tex9 = nullptr;
        }

        if( self->cubetex9 )
        {
            self->cubetex9->Release();
            self->cubetex9 = nullptr;
        }
        
        self->width  = 0;
        self->height = 0;
        
        TextureDX9Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

static void*
dx9_Texture_Map( Handle tex, unsigned level, TextureFace face )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    void*           mem  = nullptr;
    D3DLOCKED_RECT  rc   = {0};
    HRESULT         hr;
    
    
    if( self->cubetex9 )    
    {
        D3DCUBEMAP_FACES    f;

        switch( face )
        {
            case TEXTURE_FACE_LEFT   : f = D3DCUBEMAP_FACE_NEGATIVE_X ; break;
            case TEXTURE_FACE_RIGHT  : f = D3DCUBEMAP_FACE_POSITIVE_X ; break;
            case TEXTURE_FACE_FRONT  : f = D3DCUBEMAP_FACE_POSITIVE_Z ; break;
            case TEXTURE_FACE_BACK   : f = D3DCUBEMAP_FACE_NEGATIVE_Z ; break;
            case TEXTURE_FACE_TOP    : f = D3DCUBEMAP_FACE_POSITIVE_Y ; break;
            case TEXTURE_FACE_BOTTOM : f = D3DCUBEMAP_FACE_NEGATIVE_Y ; break;
        }

        hr = self->cubetex9->LockRect( f, level, &rc, NULL, 0 );

        if( SUCCEEDED(hr) )
        {
            mem = rc.pBits;

            self->mappedData  = mem;
            self->mappedLevel = level;
            self->mappedFace  = face;
            self->isMapped    = true;
        }
    }
    else
    {
        hr = self->tex9->LockRect( level, &rc, NULL, 0 );

        if( SUCCEEDED(hr) )
        {
            mem = rc.pBits;
            
            self->mappedData  = mem;
            self->mappedLevel = level;
            self->isMapped    = true;
        }
    }

    if( self->format == TEXTURE_FORMAT_A8R8G8B8 )
    {
        Size2i  ext = TextureExtents( Size2i(self->width,self->height), self->mappedLevel );
        
        _SwapRB8( self->mappedData, ext.dx*ext.dy*sizeof(uint32) );
    }
    else if( self->format == TEXTURE_FORMAT_A4R4G4B4 )
    {
        Size2i  ext = TextureExtents( Size2i(self->width,self->height), self->mappedLevel );
        
        _SwapRB4( self->mappedData, ext.dx*ext.dy*sizeof(uint16) );
    }

    return mem;
}


//------------------------------------------------------------------------------

static void
dx9_Texture_Unmap( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );

    DVASSERT(self->isMapped);

    if( self->format == TEXTURE_FORMAT_A8R8G8B8 )
    {
        Size2i  ext = TextureExtents( Size2i(self->width,self->height), self->mappedLevel );
        
        _SwapRB8( self->mappedData, ext.dx*ext.dy*sizeof(uint32) );
    }
    else if( self->format == TEXTURE_FORMAT_A4R4G4B4 )
    {
        Size2i  ext = TextureExtents( Size2i(self->width,self->height), self->mappedLevel );
        
        _SwapRB4( self->mappedData, ext.dx*ext.dy*sizeof(uint16) );
    }

    if( self->cubetex9 )
    {
        D3DCUBEMAP_FACES    f;

        switch( self->mappedLevel )
        {
            case TEXTURE_FACE_LEFT   : f = D3DCUBEMAP_FACE_NEGATIVE_X ; break;
            case TEXTURE_FACE_RIGHT  : f = D3DCUBEMAP_FACE_POSITIVE_X ; break;
            case TEXTURE_FACE_FRONT  : f = D3DCUBEMAP_FACE_POSITIVE_Z ; break;
            case TEXTURE_FACE_BACK   : f = D3DCUBEMAP_FACE_NEGATIVE_Z ; break;
            case TEXTURE_FACE_TOP    : f = D3DCUBEMAP_FACE_POSITIVE_Y ; break;
            case TEXTURE_FACE_BOTTOM : f = D3DCUBEMAP_FACE_NEGATIVE_Y ; break;
        }

        HRESULT hr = self->cubetex9->UnlockRect( f, self->mappedLevel );

        if( FAILED(hr) )
            Logger::Error( "UnlockRect failed:\n%s\n", D3D9ErrorText(hr) );
    }
    else
    {
        HRESULT hr = self->tex9->UnlockRect( self->mappedLevel );

        if( FAILED(hr) )
            Logger::Error( "UnlockRect failed:\n%s\n", D3D9ErrorText(hr) );
    }

    self->isMapped = false;
}


//------------------------------------------------------------------------------

void
dx9_Texture_Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    void*           dst  = dx9_Texture_Map( tex, level, face );
    uint32          sz   = TextureSize( self->format, self->width, self->height, level );
    
    memcpy( dst, data, sz );
    dx9_Texture_Unmap( tex );
}


//==============================================================================

namespace TextureDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Texture_Create = &dx9_Texture_Create;
    dispatch->impl_Texture_Delete = &dx9_Texture_Delete;
    dispatch->impl_Texture_Map    = &dx9_Texture_Map;
    dispatch->impl_Texture_Unmap  = &dx9_Texture_Unmap;
    dispatch->impl_Texture_Update = &dx9_Texture_Update;
}


void
SetToRHI( Handle tex, unsigned unit_i )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );

    _D3D9_Device->SetTexture( unit_i, self->basetex9 );
    self->lastUnit = unit_i;
}


void
SetAsRenderTarget( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    
    if( self->lastUnit != InvalidIndex )
    {
        _D3D9_Device->SetTexture( self->lastUnit, NULL );
        self->lastUnit = InvalidIndex;
    }

    DX9_CALL(_D3D9_Device->SetRenderTarget( 0, self->surf9 ),"SetRenderTarget");
}


}

} // namespace rhi

