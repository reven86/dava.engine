
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_FormatConversion.h"
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

    mutable IDirect3DTexture9*  rt_tex9;
    mutable IDirect3DSurface9*  rt_surf9;

    unsigned                    lastUnit;
    unsigned                    mappedLevel;
    TextureFace                 mappedFace;
    void*                       mappedData;
    unsigned                    isRenderTarget:1;
    unsigned                    isDepthStencil:1;
    unsigned                    isMapped:1;
};


TextureDX9_t::TextureDX9_t()
  : width(0),
    height(0),
    basetex9(nullptr),
    tex9(nullptr), 
    cubetex9(nullptr),
    surf9(nullptr),
    rt_tex9(nullptr),
    rt_surf9(nullptr),
    lastUnit(InvalidIndex),
    mappedData(nullptr),
    isRenderTarget(false),
    isMapped(false)
{
}

typedef ResourcePool<TextureDX9_t,RESOURCE_TEXTURE> TextureDX9Pool;
RHI_IMPL_POOL(TextureDX9_t,RESOURCE_TEXTURE);

//------------------------------------------------------------------------------

static Handle
dx9_Texture_Create( const Texture::Descriptor& desc )
{
    DVASSERT(desc.levelCount);

    Handle              handle      = InvalidHandle;
    TextureDX9_t*       tex         = nullptr;
    IDirect3DTexture9*  tex9        = nullptr;
    D3DFORMAT           fmt         = DX9_TextureFormat( desc.format );
    DWORD               usage       =(desc.isRenderTarget)  ? D3DUSAGE_RENDERTARGET  : 0;
    D3DPOOL             pool        = (desc.isRenderTarget/*  ||  options&TEXTURE_OPT_DYNAMIC*/)  ? D3DPOOL_DEFAULT  : D3DPOOL_MANAGED;
    HRESULT             hr          = E_FAIL;
    bool                auto_mip    = (desc.autoGenMipmaps)  ? true  : false;
    unsigned            mip_count   = desc.levelCount;
    bool                is_depthbuf = false;

    if( fmt == D3DFMT_D24S8  ||  fmt == D3DFMT_D32  ||  fmt == D3DFMT_D16 )
    {
        pool        = D3DPOOL_DEFAULT; 
        usage       = D3DUSAGE_DEPTHSTENCIL;
        is_depthbuf = true;
    }

//    if( options&TEXTURE_OPT_DYNAMIC )
//        usage |= D3DUSAGE_DYNAMIC;

    if( desc.isRenderTarget )
        mip_count = 1;

    
    switch( desc.type )
    {
        case TEXTURE_TYPE_2D :
        {
            DX9Command  cmd1 = { DX9Command::CREATE_TEXTURE, { desc.width, desc.height, mip_count, usage, fmt, pool, uint64_t(&tex9), 0 } };
            
            ExecDX9( &cmd1, 1 );
            hr = cmd1.retval;

            if( SUCCEEDED(hr) )
            {
                handle = TextureDX9Pool::Alloc();
                tex    = TextureDX9Pool::Get( handle );
                tex->tex9 = tex9;

                DX9Command  cmd2[] =
                {
                    { DX9Command::QUERY_INTERFACE, { uint64_t(static_cast<IUnknown*>(tex9)), uint64_t((const void*)(&IID_IDirect3DBaseTexture9)), uint64((void**)(&tex->basetex9)) } },
                    { DX9Command::SET_TEXTURE_AUTOGEN_FILTER_TYPE, { uint64_t(tex->basetex9), D3DTEXF_LINEAR } }
                };

                if( !auto_mip )
                    cmd2[1].func = DX9Command::NOP;

                ExecDX9( cmd2, countof(cmd2) );

                if( desc.isRenderTarget  ||  is_depthbuf )
                {
                    DX9Command  cmd3 = { DX9Command::GET_TEXTURE_SURFACE_LEVEl, { uint64_t(tex9), 0, uint64_t(&tex->surf9) } };

                    ExecDX9( &cmd3, 1 );
                }

                tex->width          = desc.width;
                tex->height         = desc.height;
                tex->format         = desc.format;
                tex->isDepthStencil = is_depthbuf;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }

        }   break;
        
        case TEXTURE_TYPE_CUBE :
        {
            IDirect3DCubeTexture9*  cubetex9 = nullptr;
            DX9Command              cmd1     = { DX9Command::CREATE_CUBE_TEXTURE, { desc.width, mip_count, usage, DX9_TextureFormat(desc.format), pool, uint64_t(&cubetex9), NULL } };
            
            ExecDX9( &cmd1, 1 );
            hr = cmd1.retval;
        
            if( SUCCEEDED(hr) )
            {
                handle = TextureDX9Pool::Alloc();
                tex    = TextureDX9Pool::Get( handle );

                tex->cubetex9 = cubetex9;
                tex->tex9     = nullptr;

                DX9Command  cmd2[] =
                {
                    { DX9Command::QUERY_INTERFACE, { uint64_t(static_cast<IUnknown*>(cubetex9)), uint64_t((const void*)(&IID_IDirect3DBaseTexture9)), uint64((void**)(&tex->basetex9)) } },
                    { DX9Command::SET_TEXTURE_AUTOGEN_FILTER_TYPE, { uint64_t(tex->basetex9), D3DTEXF_LINEAR } }
                };

                if( !auto_mip )
                    cmd2[1].func = DX9Command::NOP;

                ExecDX9( cmd2, countof(cmd2) );
                
                tex->width          = desc.width;
                tex->height         = desc.height;
                tex->format         = desc.format;
                tex->isDepthStencil = is_depthbuf;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }
        }   break;

    }

    tex->isRenderTarget = desc.isRenderTarget;
    
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

        DX9Command  cmd[] =
        {
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->surf9)) } },
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->basetex9)) } },
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->tex9)) } },
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->cubetex9)) } },
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->rt_surf9)) } },
            { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(self->rt_tex9)) } }
        };

        if( self->surf9 )
        {
            cmd[0].func = DX9Command::RELEASE;
            self->surf9 = nullptr;
        }

        if( self->basetex9 )
        {
            cmd[1].func = DX9Command::RELEASE;
            self->basetex9 = nullptr;
        }

        if( self->tex9 )
        {
            cmd[2].func = DX9Command::RELEASE;
            self->tex9 = nullptr;
        }

        if( self->cubetex9 )
        {
            cmd[3].func = DX9Command::RELEASE;
            self->cubetex9 = nullptr;
        }
        
        if( self->rt_surf9 )
        {
            cmd[4].func = DX9Command::RELEASE;
            self->rt_surf9 = nullptr;
        }

        if( self->rt_tex9 )
        {
            cmd[5].func = DX9Command::RELEASE;
            self->rt_tex9 = nullptr;
        }

        ExecDX9( cmd, countof(cmd) );


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
        DVASSERT(!self->isRenderTarget);
        D3DCUBEMAP_FACES    f;

        switch( face )
        {
            case TEXTURE_FACE_POSITIVE_X: f = D3DCUBEMAP_FACE_POSITIVE_X; break;
            case TEXTURE_FACE_NEGATIVE_X: f = D3DCUBEMAP_FACE_NEGATIVE_X; break;
            case TEXTURE_FACE_POSITIVE_Y: f = D3DCUBEMAP_FACE_POSITIVE_Y; break;
            case TEXTURE_FACE_NEGATIVE_Y: f = D3DCUBEMAP_FACE_NEGATIVE_Y; break;
            case TEXTURE_FACE_POSITIVE_Z: f = D3DCUBEMAP_FACE_POSITIVE_Z; break;
            case TEXTURE_FACE_NEGATIVE_Z: f = D3DCUBEMAP_FACE_NEGATIVE_Z; break;            
        }
        
        DX9Command  cmd = { DX9Command::LOCK_CUBETEXTURE_RECT, { uint64_t(self->cubetex9), f, level, uint64(&rc), NULL, 0 } };
            
        ExecDX9( &cmd, 1 );
        hr = cmd.retval;

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
        if( self->isRenderTarget )
        {
            DVASSERT(level==0);

            if( !self->rt_tex9 )
            {
                DX9Command  cmd1 = { DX9Command::CREATE_TEXTURE, { self->width, self->height, 1, 0, DX9_TextureFormat(self->format), D3DPOOL_SYSTEMMEM, uint64_t(&self->rt_tex9), 0 } };
                
                ExecDX9( &cmd1, 1 );

                if( SUCCEEDED(cmd1.retval) )
                {
                    DX9Command  cmd2 = { DX9Command::GET_TEXTURE_SURFACE_LEVEl, { uint64_t(self->rt_tex9), 0, uint64_t(&self->rt_surf9) } };
                    
                    ExecDX9( &cmd2, 1 );
                }
            }

            if( self->rt_tex9  &&  self->rt_surf9 )
            {
                DX9Command  cmd3 = { DX9Command::GET_RENDERTARGET_DATA, { uint64_t(self->surf9), uint64_t(self->rt_surf9) } };

                ExecDX9( &cmd3, 1 );

                if( cmd3.retval == D3D_OK )
                {
                    DX9Command  cmd4 = { DX9Command::LOCK_TEXTURE_RECT, { uint64_t(self->rt_tex9), level, uint64(&rc), NULL, 0 } };
                    
                    ExecDX9( &cmd4, 1 );
                    hr = cmd4.retval;

                    if( SUCCEEDED(hr) )
                    {
                        mem = rc.pBits;
            
                        self->mappedData  = mem;
                        self->mappedLevel = level;
                        self->isMapped    = true;
                    }
                }
            }
        }
        else
        {
            DX9Command  cmd = { DX9Command::LOCK_TEXTURE_RECT, { uint64_t(self->tex9), level, uint64(&rc), NULL, 0 } };
            
            ExecDX9( &cmd, 1 );
            
            if( SUCCEEDED(cmd.retval) )
            {
                mem = rc.pBits;
            
                self->mappedData  = mem;
                self->mappedLevel = level;
                self->isMapped    = true;
            }
        }
    }

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

    return mem;
}


//------------------------------------------------------------------------------

static void
dx9_Texture_Unmap( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );

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

    if( self->cubetex9 )
    {
        D3DCUBEMAP_FACES    f;

        switch( self->mappedFace )
        {
            case TEXTURE_FACE_POSITIVE_X: f = D3DCUBEMAP_FACE_POSITIVE_X; break;
            case TEXTURE_FACE_NEGATIVE_X: f = D3DCUBEMAP_FACE_NEGATIVE_X; break;
            case TEXTURE_FACE_POSITIVE_Y: f = D3DCUBEMAP_FACE_POSITIVE_Y; break;
            case TEXTURE_FACE_NEGATIVE_Y: f = D3DCUBEMAP_FACE_NEGATIVE_Y; break;
            case TEXTURE_FACE_POSITIVE_Z: f = D3DCUBEMAP_FACE_POSITIVE_Z; break;
            case TEXTURE_FACE_NEGATIVE_Z: f = D3DCUBEMAP_FACE_NEGATIVE_Z; break;
        }

        DX9Command          cmd = { DX9Command::UNLOCK_CUBETEXTURE_RECT, { uint64_t(self->cubetex9), f, self->mappedLevel } };
            
        ExecDX9( &cmd, 1 );
        
        if( FAILED(cmd.retval) )
            Logger::Error( "UnlockRect failed:\n%s\n", D3D9ErrorText(cmd.retval) );
    }
    else
    {
        IDirect3DTexture9*  tex = (self->isRenderTarget)  ? self->rt_tex9  : self->tex9;
        DX9Command          cmd = { DX9Command::UNLOCK_TEXTURE_RECT, { uint64_t(self->tex9), self->mappedLevel } };
            
        ExecDX9( &cmd, 1 );
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

void
SetAsDepthStencil( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    
    DX9_CALL(_D3D9_Device->SetDepthStencilSurface( self->surf9 ),"SetDepthStencilSurface");
}


}

} // namespace rhi

