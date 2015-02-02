
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
TextureDX9_t
{
public:

                                TextureDX9_t();


    unsigned                    _width;
    unsigned                    _height;
    IDirect3DBaseTexture9*      _basetex9;
    IDirect3DTexture9*          _tex9;    
    IDirect3DCubeTexture9*      _cubetex9;
    mutable IDirect3DSurface9*  _surf9;
    unsigned                    _is_mapped:1;
};


TextureDX9_t::TextureDX9_t()
  : _width(0),
    _height(0),
    _basetex9(0),
    _tex9(0), 
    _cubetex9(0),
    _surf9(0),
    _is_mapped(false)
{
}

typedef Pool<TextureDX9_t,RESOURCE_TEXTURE>   TextureDX9Pool;
RHI_IMPL_POOL(TextureDX9_t,RESOURCE_TEXTURE);



namespace Texture
{
//------------------------------------------------------------------------------

Handle
Create( unsigned width, unsigned height, TextureFormat format, uint32 options )
{
    Handle              handle      = InvalidHandle;
    TextureDX9_t*       tex         = 0;
    IDirect3DTexture9*  tex9        = 0;
    DWORD               usage       = 0;//(is_rt)  ? D3DUSAGE_RENDERTARGET  : 0;
    D3DPOOL             pool        = D3DPOOL_MANAGED;//(is_rt  ||  _is_dynamic)  ? D3DPOOL_DEFAULT  : D3DPOOL_MANAGED;
    HRESULT             hr          = E_FAIL;
    bool                auto_mip    = true;//(mip_count == AutoGenMipMaps)  ? true  : false;
    unsigned            mip_count   = 0;

    {
        usage       |= D3DUSAGE_AUTOGENMIPMAP;
        mip_count    = 0;
    }

    hr = _D3D9_Device->CreateTexture( width, height,
                                      mip_count,
                                      usage,
                                      D3DFMT_A8R8G8B8,
                                      pool,
                                      &tex9,
                                      NULL
                                    );
    if( SUCCEEDED(hr) )
    {
        handle = TextureDX9Pool::Alloc();
        tex    = TextureDX9Pool::Get( handle );

        tex->_tex9 = tex9;

        hr = tex9->QueryInterface( IID_IDirect3DBaseTexture9, (void**)(&tex->_basetex9) );
        if( tex->_basetex9  &&  auto_mip )
            tex->_basetex9->SetAutoGenFilterType( D3DTEXF_LINEAR );
    }
    else
    {
        Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
    }
    
    return handle;
}


//------------------------------------------------------------------------------

void
Delete( Handle tex )
{
    if( tex != InvalidHandle )
    {
        TextureDX9_t* self = TextureDX9Pool::Get( tex );

        DVASSERT(!self->_is_mapped);

        if( self->_surf9 )
        {
            self->_surf9->Release();
            self->_surf9 = 0;
        }

        if( self->_basetex9 )
        {
            self->_basetex9->Release();
            self->_basetex9 = 0;
        }

        if( self->_tex9 )
        {
            self->_tex9->Release();
            self->_tex9 = 0;
        }

        if( self->_cubetex9 )
        {
            self->_cubetex9->Release();
            self->_cubetex9 = 0;
        }
        
        self->_width    = 0;
        self->_height   = 0;
        
        TextureDX9Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

void*
Map( Handle tex, unsigned level )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    void*           mem  = 0;
    D3DLOCKED_RECT  rc   = {0};
    HRESULT         hr   = self->_tex9->LockRect( level, &rc, NULL, 0 );

    if( SUCCEEDED(hr) )
    {
        mem = rc.pBits;

///        if( pitch )
///            *pitch = rc.Pitch;
        self->_is_mapped = true;
    }

    return mem;
}


//------------------------------------------------------------------------------

void
Unmap( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );

    DVASSERT(self->_is_mapped);

    HRESULT hr = self->_tex9->UnlockRect( 0/*level*/ );

    if( FAILED(hr) )
        Logger::Error( "UnlockRect failed:\n%s\n", D3D9ErrorText(hr) );

    self->_is_mapped = false;
}

//==============================================================================
} // namespace Texture



namespace TextureDX9
{
void
SetToRHI( Handle tex, unsigned unit_i )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );

    _D3D9_Device->SetTexture( unit_i, self->_basetex9 );
}

}

} // namespace rhi

