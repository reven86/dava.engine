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
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"

    #include <string.h>


namespace rhi
{
//==============================================================================

class
TextureGLES2_t
  : public ResourceImpl<TextureGLES2_t,Texture::Descriptor>
{
public:

                    TextureGLES2_t();

    bool            Create( const Texture::Descriptor& desc, bool force_immediate=false );
    void            Destroy( bool force_immediate=false );


    unsigned        uid;
    unsigned        uid2;

    unsigned        width;
    unsigned        height;
    TextureFormat   format;
    void*           mappedData;
    uint32          mappedLevel;
    GLenum          mappedFace;
    uint32          isCubeMap:1;
    uint32          isRenderTarget:1;
    uint32          isRenderBuffer:1;
    uint32          isMapped:1;

    struct
    fbo_t
    {
        Handle  color;
        Handle  depthStencil;

        GLuint  frameBuffer;
    };
    std::vector<fbo_t>  fbo;


    SamplerState::Descriptor::Sampler   samplerState;
    uint32                              forceSetSamplerState:1;
};
RHI_IMPL_RESOURCE(TextureGLES2_t,Texture::Descriptor);

TextureGLES2_t::TextureGLES2_t()
  : uid(0),
    uid2(0),
    fbo(0),
    width(0),
    height(0),
    isCubeMap(false),
    isRenderTarget(false),
    isRenderBuffer(false),
    mappedData(nullptr),
    isMapped(false),
    forceSetSamplerState(true)
{
}


//------------------------------------------------------------------------------

bool
TextureGLES2_t::Create( const Texture::Descriptor& desc, bool force_immediate )
{
    DVASSERT(desc.levelCount);

    bool        success      = false;
    GLuint      uid[2]       = { 0, 0 };
    bool        is_depth     = desc.format == TEXTURE_FORMAT_D16  ||  desc.format == TEXTURE_FORMAT_D24S8;
    bool        need_stencil = desc.format == TEXTURE_FORMAT_D24S8;
    
    if( is_depth )
    {
        GLCommand   cmd1  = { GLCommand::GEN_RENDERBUFFERS, { uint64((need_stencil)?2:1), (uint64)(uid) } };
        #if defined(__DAVAENGINE_IPHONE__)
        int         gl_ds = GL_DEPTH24_STENCIL8_OES;
        #else
        int         gl_ds = GL_DEPTH24_STENCIL8;
        #endif
        
        ExecGL( &cmd1, 1 );

        if( cmd1.status == GL_NO_ERROR )
        {
            GLCommand   cmd2[] =
            {
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[0] } },
                { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, uint64(gl_ds), desc.width, desc.height } },
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } }
/*
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[0] } },
                { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, (need_stencil)?GL_DEPTH_COMPONENT24:GL_DEPTH_COMPONENT16, desc.width, desc.height } },
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } },
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, uid[1] } },
                { GLCommand::RENDERBUFFER_STORAGE, { GL_RENDERBUFFER, GL_STENCIL_INDEX8, desc.width, desc.height } },
                { GLCommand::BIND_RENDERBUFFER, { GL_RENDERBUFFER, 0 } },
*/            
            };
/*
            if( !need_stencil )
            {
                cmd2[3].func = GLCommand::NOP;
                cmd2[4].func = GLCommand::NOP;
                cmd2[5].func = GLCommand::NOP;
            }
*/
            ExecGL( cmd2, countof(cmd2), force_immediate );
        }
    }
    else
    {
        GLCommand   cmd1 = { GLCommand::GEN_TEXTURES, { 1, (uint64)(uid) } };

        ExecGL( &cmd1, 1, force_immediate );

        if(     cmd1.status == GL_NO_ERROR 
            &&  (desc.autoGenMipmaps  &&  !desc.isRenderTarget)
          )
        {
            GLenum      target = (desc.type == TEXTURE_TYPE_CUBE)  ? GL_TEXTURE_CUBE_MAP  : GL_TEXTURE_2D;
            GLCommand   cmd2[] =
            {
                { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0+0 } },
                { GLCommand::BIND_TEXTURE, { target, uid[0] } },
                { GLCommand::GENERATE_MIPMAP, {} },
                { GLCommand::RESTORE_TEXTURE0, {} }
            };
            
            ExecGL( cmd2, countof(cmd2), force_immediate );
        }
    }


    if( uid[0] )
    {
        this->uid            = uid[0];
        this->uid2           = uid[1];
        mappedData           = nullptr;
        width                = desc.width;
        height               = desc.height;
        format               = desc.format;
        isCubeMap            = desc.type == TEXTURE_TYPE_CUBE;
        isMapped             = false;
        isRenderTarget       = false;
        isRenderBuffer       = is_depth;
        forceSetSamplerState = true;
        
        if( desc.isRenderTarget )
        {
            isRenderTarget        = true;
            forceSetSamplerState  = false;

            GLCommand   cmd3[] =
            {
                { GLCommand::BIND_TEXTURE, { GL_TEXTURE_2D, uid[0] } },
                { GLCommand::TEX_IMAGE2D, { GL_TEXTURE_2D, 0, GL_RGBA, desc.width, desc.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0 } },
                { GLCommand::TEX_PARAMETER_I, { GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST } },
                { GLCommand::TEX_PARAMETER_I, { GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST } },
                { GLCommand::BIND_TEXTURE, { GL_TEXTURE_2D, 0 } },
                { GLCommand::RESTORE_TEXTURE0, {} }
            };

            ExecGL( cmd3, countof(cmd3), force_immediate );
        }
        else
        {
            isRenderTarget = false;
        }

        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void
TextureGLES2_t::Destroy( bool force_immediate )
{
    GLCommand       cmd[16];
    unsigned        cmd_cnt = 1;

    if( isRenderTarget )
    {
        DVASSERT(fbo.size() <= countof(cmd)-1);
        for( unsigned i=0; i!=fbo.size(); ++i )
        {
            cmd[1+i].func   = GLCommand::DELETE_FRAMEBUFFERS;
            cmd[1+i].arg[0] = 1;
            cmd[1+i].arg[1] = uint64(&(fbo[i].frameBuffer));
        }

        cmd_cnt += fbo.size();
        fbo.clear();
    }
    else if( isRenderBuffer )
    {
        cmd[0].func   = GLCommand::DELETE_RENDERBUFFERS;
        cmd[0].arg[0] = 1;
        cmd[0].arg[1] = uint64(&(uid));

        if( uid2 )
        {
            cmd[1].func   = GLCommand::DELETE_RENDERBUFFERS;
            cmd[1].arg[0] = 1;
            cmd[1].arg[1] = uint64(&(uid2));
        }
    }
    else
    {
        cmd[0].func   = GLCommand::DELETE_TEXTURES;
        cmd[0].arg[0] = 1;
        cmd[0].arg[1] = uint64(&(uid));
    }

    ExecGL( cmd, cmd_cnt );



    DVASSERT(!isMapped);
        
    if( mappedData )
    {
        ::free( mappedData );
            
        mappedData = nullptr;
        width      = 0;
        height     = 0;
    }
}

typedef ResourcePool<TextureGLES2_t,RESOURCE_TEXTURE,Texture::Descriptor,true>   TextureGLES2Pool;
RHI_IMPL_POOL(TextureGLES2_t,RESOURCE_TEXTURE,Texture::Descriptor,true);


//------------------------------------------------------------------------------

static void
gles2_Texture_Delete( Handle tex )
{
    if( tex != InvalidHandle )
    {
        TextureGLES2_t* self = TextureGLES2Pool::Get( tex );

        self->MarkRestored();
        self->Destroy();
        TextureGLES2Pool::Free( tex );
    }
}


//------------------------------------------------------------------------------

static Handle
gles2_Texture_Create( const Texture::Descriptor& desc )
{
    Handle          handle = TextureGLES2Pool::Alloc();
    TextureGLES2_t* tex    = TextureGLES2Pool::Get( handle );

    if( tex->Create( desc ) )
    {
        tex->UpdateCreationDesc( desc );
    }
    else
    {
        TextureGLES2Pool::Free( handle );
        handle = InvalidHandle;
    }
    
    return handle;
}


//------------------------------------------------------------------------------

static void*
gles2_Texture_Map( Handle tex, unsigned level, TextureFace face )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    void*              mem = nullptr;
    uint32 textureDataSize = TextureSize(self->format, self->width, self->height, level);
    
    DVASSERT(!self->isRenderBuffer);
    DVASSERT(!self->isMapped);

    self->mappedData = ::realloc( self->mappedData, textureDataSize );
    
    if( self->mappedData )
    {
        if( self->isRenderTarget )
        {
            DVASSERT(level==0);
            DVASSERT(self->fbo.size())
            GLenum      target = (self->isCubeMap)  ? GL_TEXTURE_CUBE_MAP  : GL_TEXTURE_2D;
            GLCommand   cmd[]  =
            {
                { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, self->fbo[0].frameBuffer } },
                { GLCommand::READ_PIXELS, { 0,0,self->width,self->height,GL_RGBA,GL_UNSIGNED_BYTE,uint64(self->mappedData) } },
                { GLCommand::BIND_FRAMEBUFFER, { GL_FRAMEBUFFER, _GLES2_Binded_FrameBuffer } },
            };

            ExecGL( cmd, countof(cmd) );            
            
            mem               = self->mappedData;
            self->mappedLevel = 0;
            self->isMapped    = true;
        }
        else
        {
            mem               = self->mappedData;
            self->mappedLevel = level;
            self->isMapped    = true;

            switch( face )
            {
                case TEXTURE_FACE_POSITIVE_X: self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
                case TEXTURE_FACE_NEGATIVE_X: self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
                case TEXTURE_FACE_POSITIVE_Y: self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
                case TEXTURE_FACE_NEGATIVE_Y: self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
                case TEXTURE_FACE_POSITIVE_Z: self->mappedFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
                case TEXTURE_FACE_NEGATIVE_Z: self->mappedFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
            }
        }
    }
    
    if( self->format == TEXTURE_FORMAT_R4G4B4A4 )
    {
        _FlipRGBA4_ABGR4( self->mappedData, textureDataSize );
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _ABGR1555toRGBA5551(self->mappedData, textureDataSize);
    }

    return mem;
}


//------------------------------------------------------------------------------

static void
gles2_Texture_Unmap( Handle tex )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    Size2i          sz   = TextureExtents( Size2i(self->width,self->height), self->mappedLevel );
    GLint           int_fmt;
    GLint           fmt;
    GLenum          type;
    bool            compressed;
    
    uint32 textureDataSize = TextureSize(self->format, sz.dx, sz.dy);
    GetGLTextureFormat( self->format, &int_fmt, &fmt, &type, &compressed );

    DVASSERT(self->isMapped);
    if( self->format == TEXTURE_FORMAT_R4G4B4A4 )
    {
        _FlipRGBA4_ABGR4( self->mappedData, textureDataSize );
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _RGBA5551toABGR1555(self->mappedData, textureDataSize);
    }

    GLenum      target = (self->isCubeMap)  ? GL_TEXTURE_CUBE_MAP  : GL_TEXTURE_2D;
    GLCommand   cmd[]  =
    {
        { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0+0 } },
        { GLCommand::BIND_TEXTURE, { target, self->uid } },
        { GLCommand::TEX_IMAGE2D, { target, self->mappedLevel, uint64(int_fmt), uint64(sz.dx), uint64(sz.dy), 0, uint64(fmt), type, uint64(textureDataSize), (uint64)(self->mappedData), compressed } },
        { GLCommand::RESTORE_TEXTURE0, {} }
    };

    ExecGL( cmd, countof(cmd) );

    self->isMapped = false;
}


//------------------------------------------------------------------------------

void
gles2_Texture_Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    Size2i          sz   = TextureExtents( Size2i(self->width,self->height), level );
    GLint           int_fmt;
    GLint           fmt;
    GLenum          type;
    bool            compressed;
    uint32          textureDataSize = TextureSize( self->format, sz.dx, sz.dy );
    
    DVASSERT(!self->isRenderBuffer);
    DVASSERT(!self->isMapped);

    GetGLTextureFormat( self->format, &int_fmt, &fmt, &type, &compressed );
    
    if( self->format == TEXTURE_FORMAT_R4G4B4A4 || self->format == TEXTURE_FORMAT_R5G5B5A1 )
    {
        gles2_Texture_Map( tex, level, face );
        memcpy( self->mappedData, data, textureDataSize );
        gles2_Texture_Unmap( tex );
    }
    else
    {
        GLenum      ttarget = (self->isCubeMap)  ? GL_TEXTURE_CUBE_MAP  : GL_TEXTURE_2D;
        GLenum      target  = GL_TEXTURE_2D;
        
        if( self->isCubeMap )
        {
            switch( face )
            {
                case TEXTURE_FACE_POSITIVE_X: target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
                case TEXTURE_FACE_NEGATIVE_X: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
                case TEXTURE_FACE_POSITIVE_Y: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
                case TEXTURE_FACE_NEGATIVE_Y: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
                case TEXTURE_FACE_POSITIVE_Z: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
                case TEXTURE_FACE_NEGATIVE_Z: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
            }
        }

        GLCommand   cmd[]  =
        {        
            { GLCommand::SET_ACTIVE_TEXTURE, { GL_TEXTURE0+0 } },
            { GLCommand::BIND_TEXTURE, { ttarget, self->uid } },
            { GLCommand::TEX_IMAGE2D, { target, uint64(level), uint64(int_fmt), uint64(sz.dx), uint64(sz.dy), 0, uint64(fmt), type, uint64(textureDataSize), (uint64)(data), compressed } },
            { GLCommand::RESTORE_TEXTURE0, {} }
        };

        ExecGL( cmd, countof(cmd) );
    }
}


//------------------------------------------------------------------------------

static bool
gles2_Texture_NeedRestore( Handle tex )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    
    return self->NeedRestore();
}



//==============================================================================

struct
SamplerStateGLES2_t
{
    SamplerState::Descriptor::Sampler   fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32                              fragmentSamplerCount;
    SamplerState::Descriptor::Sampler   vertexSampler[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    uint32                              vertexSamplerCount;
};

typedef ResourcePool<SamplerStateGLES2_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false>    SamplerStateGLES2Pool;
RHI_IMPL_POOL(SamplerStateGLES2_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false);
static const SamplerStateGLES2_t*                           _CurSamplerState = nullptr;


//------------------------------------------------------------------------------

static Handle
gles2_SamplerState_Create( const SamplerState::Descriptor& desc )
{
    Handle                  handle = SamplerStateGLES2Pool::Alloc();
    SamplerStateGLES2_t*    state  = SamplerStateGLES2Pool::Get( handle );
    
    state->fragmentSamplerCount = desc.fragmentSamplerCount;
    for( unsigned i=0; i!=desc.fragmentSamplerCount; ++i )    
    {
        state->fragmentSampler[i] = desc.fragmentSampler[i];
    }
    
    state->vertexSamplerCount = desc.vertexSamplerCount;
    for( unsigned i=0; i!=desc.vertexSamplerCount; ++i )    
    {
        state->vertexSampler[i] = desc.vertexSampler[i];
    }
    
    // force no-filtering on vertex-textures
    for( uint32 s=0; s!=MAX_VERTEX_TEXTURE_SAMPLER_COUNT; ++s )
    {
        state->vertexSampler[s].minFilter = TEXFILTER_NEAREST;
        state->vertexSampler[s].magFilter = TEXFILTER_NEAREST;
        state->vertexSampler[s].mipFilter = TEXMIPFILTER_NONE;
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
gles2_SamplerState_Delete( Handle hstate )
{
    SamplerStateGLES2_t*    state  = SamplerStateGLES2Pool::Get( hstate );
    
    if( _CurSamplerState == state )
        _CurSamplerState = nullptr;

    SamplerStateGLES2Pool::Free( hstate );
}


//==============================================================================

namespace SamplerStateGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_SamplerState_Create = &gles2_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &gles2_SamplerState_Delete;
}

void
SetToRHI( Handle hstate )
{
    SamplerStateGLES2_t*    state  = SamplerStateGLES2Pool::Get( hstate );

    _CurSamplerState = state;
}

}


//==============================================================================

static GLenum
_TextureFilter( TextureFilter filter )
{
    GLenum   f = GL_LINEAR;

    switch( filter )
    {
        case TEXFILTER_NEAREST  : f = GL_NEAREST; break;
        case TEXFILTER_LINEAR   : f = GL_LINEAR; break;
    }

    return f;
}


//------------------------------------------------------------------------------

static GLenum
_TextureMipFilter( TextureMipFilter filter )
{
    GLenum   f = GL_LINEAR_MIPMAP_LINEAR;
    
    switch( filter )
    {
        case TEXMIPFILTER_NONE    : f = GL_NEAREST_MIPMAP_NEAREST; break;
        case TEXMIPFILTER_NEAREST : f = GL_LINEAR_MIPMAP_NEAREST; break;
        case TEXMIPFILTER_LINEAR  : f = GL_LINEAR_MIPMAP_LINEAR; break;
    }
    
    return f;
}


//------------------------------------------------------------------------------

static GLenum
_AddrMode( TextureAddrMode mode )
{
    GLenum   m = GL_REPEAT;
    
    switch( mode )
    {
        case TEXADDR_WRAP   : m = GL_REPEAT; break;
        case TEXADDR_CLAMP  : m = GL_CLAMP_TO_EDGE; break;
        case TEXADDR_MIRROR : m = GL_MIRRORED_REPEAT; break;
    }
    
    return m;
}



//==============================================================================

namespace TextureGLES2
{ 

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Texture_Create       = &gles2_Texture_Create;
    dispatch->impl_Texture_Delete       = &gles2_Texture_Delete;
    dispatch->impl_Texture_Map          = &gles2_Texture_Map;
    dispatch->impl_Texture_Unmap        = &gles2_Texture_Unmap;
    dispatch->impl_Texture_Update       = &gles2_Texture_Update;
    dispatch->impl_Texture_NeedRestore  = &gles2_Texture_NeedRestore;
}

void
SetToRHI( Handle tex, unsigned unit_i, uint32 base_i )
{
    TextureGLES2_t* self        = TextureGLES2Pool::Get( tex );
    bool            fragment    = base_i != InvalidIndex;
    uint32          sampler_i   = (base_i == InvalidIndex)  ? unit_i  :  base_i+unit_i;
    GLenum          target      = (self->isCubeMap)  ? GL_TEXTURE_CUBE_MAP  : GL_TEXTURE_2D;

    const SamplerState::Descriptor::Sampler* sampler = (fragment) 
                                                        ? _CurSamplerState->fragmentSampler + unit_i
                                                        : _CurSamplerState->vertexSampler + unit_i;

    GL_CALL(glActiveTexture( GL_TEXTURE0+sampler_i ));
    GL_CALL(glBindTexture( target, self->uid ));
    
    if( sampler_i == 0 )
    {
        _GLES2_LastSetTex0       = self->uid;
        _GLES2_LastSetTex0Target = target;
    }

    if(     _CurSamplerState
        &&  (self->forceSetSamplerState  ||  memcmp( &(self->samplerState), sampler, sizeof(rhi::SamplerState::Descriptor::Sampler)) )     
        &&  !self->isRenderBuffer
       )
    {
        if( sampler->mipFilter != TEXMIPFILTER_NONE )
        {
            GL_CALL(glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _TextureMipFilter( TextureMipFilter(sampler->mipFilter) ) ));
        }
        else
        {
            GL_CALL(glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _TextureFilter( TextureFilter(sampler->minFilter) ) ));
        }
        
        GL_CALL(glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _TextureFilter( TextureFilter(sampler->magFilter) ) ));

        GL_CALL(glTexParameteri( target, GL_TEXTURE_WRAP_S, _AddrMode( TextureAddrMode(sampler->addrU) ) ));
        GL_CALL(glTexParameteri( target, GL_TEXTURE_WRAP_T, _AddrMode( TextureAddrMode(sampler->addrV) ) ));
        
        self->samplerState          = *sampler;
        self->forceSetSamplerState  = false;
    }
}

void
SetAsRenderTarget( Handle tex, Handle depth )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    GLuint          fb   = 0;

    for( unsigned i=0; i!=self->fbo.size(); ++i )
    {
        if( self->fbo[i].color == tex  &&  self->fbo[i].depthStencil == depth )
        {
            fb = self->fbo[i].frameBuffer;
            break;
        }
    }

    if( !fb )
    {            
        glGenFramebuffers( 1, &fb );
            
        if( fb )
        {
            TextureGLES2_t* ds   = (depth != InvalidHandle  &&  depth != DefaultDepthBuffer)  ? TextureGLES2Pool::Get( depth )  : nullptr;
            GLenum          b[1] = { GL_COLOR_ATTACHMENT0 };
                
            glBindFramebuffer( GL_FRAMEBUFFER, fb );
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->uid, 0 );
            if( ds )
            {
                #if defined(__DAVAENGINE_IPHONE__)
                int att = GL_DEPTH_STENCIL_OES;
                #else
                int att = GL_DEPTH_STENCIL_ATTACHMENT;
                #endif
                
                glFramebufferRenderbuffer( GL_FRAMEBUFFER, att, GL_RENDERBUFFER, ds->uid );
/*
                glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ds->uid );
                
                if( ds->uid2 )
                    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ds->uid2 );
*/            
            }
            #if defined __DAVAENGINE_IPHONE__
            #else
            glDrawBuffers( 1, b );
            #endif
            
            int status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            
            if( status == GL_FRAMEBUFFER_COMPLETE )
            {
                TextureGLES2_t::fbo_t   fbo = { tex, depth, fb };

                self->fbo.push_back( fbo );
            }
            else
            {
                Logger::Error( "glCheckFramebufferStatus= %08X", status );
                DVASSERT(status == GL_FRAMEBUFFER_COMPLETE);
            }
        }
    }

    glBindFramebuffer( GL_FRAMEBUFFER, fb );
    _GLES2_Binded_FrameBuffer = fb;
}

Size2i
Size( Handle tex )
{
    TextureGLES2_t* self = TextureGLES2Pool::Get( tex );
    
    return Size2i( self->width, self->height );
}

void
ReCreateAll()
{
    TextureGLES2Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return TextureGLES2_t::NeedRestoreCount();
}


} // namespace TextureGLES2

//==============================================================================
} // namespace rhi

