
    #include "rhi_ProgGLES2.h"
    #include "../rhi_VertexLayout.h"
    #include "../rhi_Base.h"
    #include "../rhi_Pool.h"

    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"

    #include <stdio.h>
    #include <string.h>


namespace rhi
{
//==============================================================================

typedef Pool<ProgGLES2::ConstBuf>   ConstBufGLES2Pool;

//==============================================================================

ProgGLES2::ProgGLES2( ProgType t )
  : type(t),
    shader(0),
    texunitInited(false)
{
    for( unsigned i=0; i!=MaxConstBufCount; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }
}


//------------------------------------------------------------------------------

ProgGLES2::~ProgGLES2()
{
}


//------------------------------------------------------------------------------

bool
ProgGLES2::Construct( const char* src_code )
{
    bool        success = false;
    int         stype   = (type==PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    unsigned    s       = glCreateShader( stype );

    if( s )
    {
        int status = 0;

        GL_CALL(glShaderSource( s, 1, &src_code, 0 ));
        GL_CALL(glCompileShader( s ));
        GL_CALL(glGetShaderiv( s, GL_COMPILE_STATUS, &status ));

        if( status ) 
        {
            shader  = s;
            success = true;            
        }
        else
        {
            char    info[1024] = "";

            glGetShaderInfoLog( s, countof(info), 0, info );
            Logger::Error( "%sprog-compile failed:", (type==PROG_VERTEX) ? "v" : "f" );
            Logger::Info( info );
        }
    }
    else
    {
        Logger::Error( "failed to create shader\n" );
    }

    return success;
}


//------------------------------------------------------------------------------

void
ProgGLES2::Destroy()
{
}


//------------------------------------------------------------------------------

void
ProgGLES2::GetProgParams( unsigned progUid )
{
    GLint   cnt = 0;
    
    glGetProgramiv( progUid, GL_ACTIVE_UNIFORMS, &cnt );

    for( unsigned i=0; i!=MaxConstBufCount; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }
    
    for( unsigned u=0; u!=cnt; ++u )
    {    
        char    name[64];
        GLsizei length;
        GLint   size;
        GLenum  type;
        
        glGetActiveUniform( progUid, u, sizeof(name)-1, &length, &size, &type, name );
        DVASSERT(glGetError() == GL_NO_ERROR);
        
        for( unsigned i=0; i!=MaxConstBufCount; ++i )
        {
            char    n[16];   sprintf( n, "%s_Buffer%u[0]", (type == PROG_VERTEX)?"VP":"FP", i );
            
            if( !strcmp( name, n ) )
            {            
                int     loc = glGetUniformLocation( progUid, name );

//Logger::Info( "  {%u}  %s -> %i", shader, name, loc );                
                if( loc != -1 )
                {
                    cbuf[i].location = loc;
                    cbuf[i].count    = size;
                    break;
                }
            }
        }
    }

    for( unsigned i=0; i!=countof(texunitLoc); ++i )
    {
        char    name[16];   _snprintf( name, countof(name), "Texture%u", i );
        int     loc = glGetUniformLocation( progUid, name );

        texunitLoc[i] = (loc != -1)  ? loc  : InvalidIndex;
    }

    texunitInited = false;
}


//------------------------------------------------------------------------------

unsigned
ProgGLES2::ConstBufferCount() const
{
    return countof(cbuf);
}


//------------------------------------------------------------------------------

Handle
ProgGLES2::InstanceConstBuffer( unsigned bufIndex )
{
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].location != InvalidIndex )
    {
        handle = ConstBufGLES2Pool::Alloc();

        ConstBuf*   cb = ConstBufGLES2Pool::Get( handle );

        if( !cb->construct( cbuf[bufIndex].location, cbuf[bufIndex].count ) )
        {
            ConstBufGLES2Pool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void
ProgGLES2::SetToRHI() const
{

    if( !texunitInited )
    {
        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            if( texunitLoc[i] != -1 )
                GL_CALL(glUniform1i( texunitLoc[i], i ));
        }

        texunitInited = true;
    }
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::construct( unsigned loc, unsigned cnt )
{
    _location   = loc;
    _count      = cnt;
//    _data       = (float*)(VidMem()->alloc_aligned( cnt*4*sizeof(float), 16 ));
    _data       = (float*)(malloc( cnt*4*sizeof(float) ));
    _is_dirty   = true;

    return true;
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::destroy()
{
    if( _data )
    {
//        VidMem()->free( _data );
        free( _data );
        _data     = 0;
        _location = -1;
        _count    = 0;
    }
}


//------------------------------------------------------------------------------

unsigned    
ProgGLES2::ConstBuf::const_count() const
{
    return _count;
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::set_const( unsigned const_i, unsigned const_count, const float* data )
{
    bool    success = false;

    if( const_i + const_count <= _count )
    {
        memcpy( _data + const_i*4, data, const_count*4*sizeof(float) );
        _is_dirty = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::set_to_rhi() const
{
    if( _data  &&  _is_dirty )
    {
        GL_CALL(glUniform4fv( _location, _count, _data ));
        _is_dirty = false;
    }
}


//------------------------------------------------------------------------------

unsigned
ProgGLES2::ShaderUid() const
{
    return shader;
}


//------------------------------------------------------------------------------

unsigned        
ProgGLES2::TextureLocation( unsigned texunit_i ) const
{
    return texunitLoc[texunit_i];
}



//------------------------------------------------------------------------------

namespace ConstBuffer
{

unsigned
ConstCount( Handle cb )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->const_count();
}


//------------------------------------------------------------------------------

bool
SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->set_const( const_i, const_count, data );
}

}


//------------------------------------------------------------------------------

namespace ConstBufferGLES2
{

void
SetToRHI( const Handle cb )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    self->set_to_rhi();
}

}


//==============================================================================
} // namespace rhi

