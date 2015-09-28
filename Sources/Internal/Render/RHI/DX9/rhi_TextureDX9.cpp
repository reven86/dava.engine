/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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
  : public ResourceImpl<TextureDX9_t,Texture::Descriptor>
{
public:

                                TextureDX9_t();

    bool                        Create( const Texture::Descriptor& desc, bool force_immediate=false );
    void                        Destroy( bool force_immediate=false);

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
RHI_IMPL_RESOURCE(TextureDX9_t,Texture::Descriptor);


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


//------------------------------------------------------------------------------

bool
TextureDX9_t::Create( const Texture::Descriptor& desc, bool force_immediate )
{
    DVASSERT(desc.levelCount);
    bool                success     = false;
    D3DFORMAT           fmt         = DX9_TextureFormat( desc.format );
    DWORD               usage       = (desc.isRenderTarget)  ? D3DUSAGE_RENDERTARGET  : 0;
    D3DPOOL             pool        = (desc.isRenderTarget)  ? D3DPOOL_DEFAULT  : D3DPOOL_MANAGED;
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

    if( desc.isRenderTarget )
        mip_count = 1;

    
    switch( desc.type )
    {
        case TEXTURE_TYPE_2D :
        {
            unsigned    cmd1_cnt = 1;
            DX9Command  cmd1[32] = 
            { 
                DX9Command::CREATE_TEXTURE, { desc.width, desc.height, mip_count, usage, fmt, pool, uint64_t(&tex9), 0 } 
            };

            DVASSERT(desc.levelCount <= countof(desc.initialData));    
            for( unsigned m=0; m!=desc.levelCount; ++m )
            {
                DX9Command*  cmd = cmd1 + cmd1_cnt;

                if( desc.initialData[m] )
                {
                    Size2i  sz      = TextureExtents( Size2i(desc.width,desc.height), m );
                    void*   data    = desc.initialData[m];
                    uint32  data_sz = TextureSize( desc.format, sz.dx, sz.dy);
                    
                    cmd->func   = DX9Command::UPDATE_TEXTURE_LEVEL;
                    cmd->arg[0] = uint64_t(&tex9);
                    cmd->arg[1] = m;
                    cmd->arg[2] = (uint64)(data);
                    cmd->arg[3] = data_sz;
                    
                    if( desc.format == TEXTURE_FORMAT_R8G8B8A8 )
                        _SwapRB8( data, data_sz );
                    else if( desc.format == TEXTURE_FORMAT_R4G4B4A4 )
                        _SwapRB4( data, data_sz );
                    else if (desc.format == TEXTURE_FORMAT_R5G5B5A1)
                        _SwapRB5551( data, data_sz );

                    ++cmd1_cnt;
                }
                else
                {
                    break;
                }
            }
            
            ExecDX9( cmd1, cmd1_cnt, force_immediate );
            hr = cmd1[0].retval;

            if( SUCCEEDED(hr) )
            {
                DX9Command  cmd2[] =
                {
                    { DX9Command::QUERY_INTERFACE, { uint64_t(static_cast<IUnknown*>(tex9)), uint64_t((const void*)(&IID_IDirect3DBaseTexture9)), uint64((void**)(&basetex9)) } },
                    { DX9Command::SET_TEXTURE_AUTOGEN_FILTER_TYPE, { uint64_t(basetex9), D3DTEXF_LINEAR } }
                };

                if( !auto_mip )
                    cmd2[1].func = DX9Command::NOP;

                ExecDX9( cmd2, countof(cmd2), force_immediate );

                if( desc.isRenderTarget  ||  is_depthbuf )
                {
                    DX9Command  cmd3 = { DX9Command::GET_TEXTURE_SURFACE_LEVEl, { uint64_t(tex9), 0, uint64_t(&surf9) } };

                    ExecDX9( &cmd3, 1, force_immediate );
                }

                width          = desc.width;
                height         = desc.height;
                format         = desc.format;
                isDepthStencil = is_depthbuf;

                success        = true;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }
        }   break;
        
        case TEXTURE_TYPE_CUBE :
        {
            uint32      cmd1_cnt  = 1;
            DX9Command  cmd1[128] = 
            { 
                { DX9Command::CREATE_CUBE_TEXTURE, { desc.width, mip_count, usage, DX9_TextureFormat(desc.format), pool, uint64_t(&cubetex9), NULL } }
            };
            
            DVASSERT(desc.levelCount*6 <= countof(desc.initialData));
            TextureFace         face[]     = { TEXTURE_FACE_POSITIVE_X, TEXTURE_FACE_NEGATIVE_X, TEXTURE_FACE_POSITIVE_Y, TEXTURE_FACE_NEGATIVE_Y, TEXTURE_FACE_POSITIVE_Z, TEXTURE_FACE_NEGATIVE_Z };
            D3DCUBEMAP_FACES    d3d_face[] = { D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X, D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y, D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z };
            
            for( unsigned f=0; f!=countof(face); ++f )
            {
                for( unsigned m=0; m!=desc.levelCount; ++m )
                {
                    DX9Command* cmd  = cmd1 + cmd1_cnt;
                    void*       data = desc.initialData[f*desc.levelCount+m];

                    if( data )
                    {
                        Size2i  sz      = TextureExtents( Size2i(desc.width,desc.height), m );
                        uint32  data_sz = TextureSize( desc.format, sz.dx, sz.dy);

                        cmd->func   = DX9Command::UPDATE_CUBETEXTURE_LEVEL;
                        cmd->arg[0] = uint64_t(&cubetex9);
                        cmd->arg[1] = m;
                        cmd->arg[2] = d3d_face[f];
                        cmd->arg[3] = (uint64)(data);
                        cmd->arg[4] = data_sz;
                    
                        if( desc.format == TEXTURE_FORMAT_R8G8B8A8 )
                            _SwapRB8( data, data_sz );
                        else if( desc.format == TEXTURE_FORMAT_R4G4B4A4 )
                            _SwapRB4( data, data_sz );
                        else if (desc.format == TEXTURE_FORMAT_R5G5B5A1)
                            _SwapRB5551( data, data_sz );

                        ++cmd1_cnt;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            
            ExecDX9( cmd1, cmd1_cnt, force_immediate );
            hr = cmd1[0].retval;
        
            if( SUCCEEDED(hr) )
            {
                tex9 = nullptr;

                DX9Command  cmd2[] =
                {
                    { DX9Command::QUERY_INTERFACE, { uint64_t(static_cast<IUnknown*>(cubetex9)), uint64_t((const void*)(&IID_IDirect3DBaseTexture9)), uint64((void**)(&basetex9)) } },
                    { DX9Command::SET_TEXTURE_AUTOGEN_FILTER_TYPE, { uint64_t(basetex9), D3DTEXF_LINEAR } }
                };

                if( !auto_mip )
                    cmd2[1].func = DX9Command::NOP;

                ExecDX9( cmd2, countof(cmd2) );
                
                width          = desc.width;
                height         = desc.height;
                format         = desc.format;
                isDepthStencil = is_depthbuf;
                
                success        = true;
            }
            else
            {
                Logger::Error( "failed to create texture:\n%s\n", D3D9ErrorText(hr) );
            }
        }   break;
    }

    isRenderTarget = desc.isRenderTarget;


    return success;
}


//------------------------------------------------------------------------------

void
TextureDX9_t::Destroy( bool force_immediate )
{
    DVASSERT(!isMapped);

    DX9Command  cmd[] =
    {
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(surf9)) } },
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(basetex9)) } },
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(tex9)) } },
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(cubetex9)) } },
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(rt_surf9)) } },
        { DX9Command::NOP, { uint64_t(static_cast<IUnknown*>(rt_tex9)) } }
    };

    if( surf9 )
    {
        cmd[0].func = DX9Command::RELEASE;
        surf9 = nullptr;
    }

    if( basetex9 )
    {
        cmd[1].func = DX9Command::RELEASE;
        basetex9 = nullptr;
    }

    if( tex9 )
    {
        cmd[2].func = DX9Command::RELEASE;
        tex9 = nullptr;
    }

    if( cubetex9 )
    {
        cmd[3].func = DX9Command::RELEASE;
        cubetex9 = nullptr;
    }
        
    if( rt_surf9 )
    {
        cmd[4].func = DX9Command::RELEASE;
        rt_surf9 = nullptr;
    }

    if( rt_tex9 )
    {
        cmd[5].func = DX9Command::RELEASE;
        rt_tex9 = nullptr;
    }

    ExecDX9( cmd, countof(cmd), force_immediate );


    width  = 0;
    height = 0;
}

typedef ResourcePool<TextureDX9_t,RESOURCE_TEXTURE,Texture::Descriptor,true> TextureDX9Pool;
RHI_IMPL_POOL(TextureDX9_t,RESOURCE_TEXTURE,Texture::Descriptor,true);



//------------------------------------------------------------------------------

static Handle
dx9_Texture_Create( const Texture::Descriptor& desc )
{
    Handle          handle = TextureDX9Pool::Alloc();
    TextureDX9_t*   tex    = TextureDX9Pool::Get( handle );

    if( tex->Create( desc ) )
    {
        tex->UpdateCreationDesc( desc );
    }
    else
    {
        TextureDX9Pool::Free( handle );
        handle = InvalidHandle;
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

        self->MarkRestored();
        self->Destroy();        
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
    self->MarkRestored();
}


//------------------------------------------------------------------------------

static void
dx9_Texture_Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    void*           dst  = dx9_Texture_Map( tex, level, face );
    uint32          sz   = TextureSize( self->format, self->width, self->height, level );
    
    memcpy( dst, data, sz );
    dx9_Texture_Unmap( tex );
}


//------------------------------------------------------------------------------

static bool
dx9_Texture_NeedRestore( Handle tex )
{
    TextureDX9_t*   self = TextureDX9Pool::Get( tex );
    
    return self->NeedRestore();
}


//==============================================================================

namespace TextureDX9
{

void
Init( uint32 maxCount )
{
    TextureDX9Pool::Reserve( maxCount );
}

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Texture_Create       = &dx9_Texture_Create;
    dispatch->impl_Texture_Delete       = &dx9_Texture_Delete;
    dispatch->impl_Texture_Map          = &dx9_Texture_Map;
    dispatch->impl_Texture_Unmap        = &dx9_Texture_Unmap;
    dispatch->impl_Texture_Update       = &dx9_Texture_Update;
    dispatch->impl_Texture_NeedRestore  = &dx9_Texture_NeedRestore;
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

void
ReleaseAll()
{
    for( TextureDX9Pool::Iterator t=TextureDX9Pool::Begin(),t_end=TextureDX9Pool::End(); t!=t_end; ++t )
    {
        t->Destroy( true );
    }
}

void
ReCreateAll()
{
    TextureDX9Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return TextureDX9_t::NeedRestoreCount();
}

}

} // namespace rhi

