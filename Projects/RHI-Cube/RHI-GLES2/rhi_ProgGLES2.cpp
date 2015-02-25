
    #include "rhi_ProgGLES2.h"
    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
    #include "../RHI/rhi_RingBuffer.h"

    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Base/BaseTypes.h"

    #include "rhi_GLES2.h"
    #include "_gl.h"

    #include <stdio.h>
    #include <string.h>


namespace rhi
{
//==============================================================================

typedef Pool<ProgGLES2::ConstBuf,RESOURCE_CONST_BUFFER>   ConstBufGLES2Pool;
RHI_IMPL_POOL(ProgGLES2::ConstBuf,RESOURCE_CONST_BUFFER);

static RingBuffer   DefaultConstRingBuffer;


//==============================================================================

ProgGLES2::ProgGLES2( ProgType t )
  : type(t),
    shader(0),
    texunitInited(false)
{
    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
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
ProgGLES2::Construct( const char* srcCode )
{
    bool        success = false;
    int         stype   = (type==PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLCommand   cmd1    = { GLCommand::CREATE_SHADER, { uint64(stype) } };

    ExecGL( &cmd1, 1 );

    if( cmd1.result )
    {
        unsigned    s           = cmd1.result;
        int         status;
        char        info[1024]  = "";
        GLCommand   cmd2[]      =
        {
            { GLCommand::SHADER_SOURCE, { s, 1, uint64(&srcCode), 0 } },
            { GLCommand::COMPILE_SHADER, { s } },
            { GLCommand::GET_SHADER_IV, { s, GL_COMPILE_STATUS, uint64(&status) } },
            { GLCommand::GET_SHADER_INFO_LOG, { s, countof(info), 0, uint64(info) } }
        };

        ExecGL( cmd2, countof(cmd2) );

        if( status )
        {
            shader  = s;
            success = true;            
        }
        else
        {
            Logger::Error( "%sprog-compile failed:", (type==PROG_VERTEX) ? "v" : "f" );
            Logger::Info( info );
        }
    }
    
    return success;

/*
    bool        success = false;
    int         stype   = (type==PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    unsigned    s       = glCreateShader( stype );

    if( s )
    {
        int status = 0;

        GL_CALL(glShaderSource( s, 1, &srcCode, 0 ));
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
*/
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
#if DV_USE_UNIFORMBUFFER_OBJECT
    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        char    name[32];   sprintf( name, "%s_Buffer%u_Block", (type == PROG_VERTEX)?"VP":"FP", i );
        GLuint  loc         = glGetUniformBlockIndex( progUid, name );

        if( loc != GL_INVALID_INDEX )
        {
            GLint   sz;

            glGetActiveUniformBlockiv( progUid, loc, GL_UNIFORM_BLOCK_DATA_SIZE, &sz );
            GL_CALL(glUniformBlockBinding( progUid, loc, loc ));

            cbuf[i].location = loc;
            cbuf[i].count    = sz / (4*sizeof(float));
        }
        else
        {
            cbuf[i].location = InvalidIndex;
            cbuf[i].count    = 0;
        }
    }
#else

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }

    GLint       cnt  = 0;
    GLCommand   cmd1 = { GLCommand::GET_PROGRAM_IV, { progUid, GL_ACTIVE_UNIFORMS, uint64(&cnt) } };
    
    ExecGL( &cmd1, 1 );

    for( unsigned u=0; u!=cnt; ++u )
    {    
        char        name[64];
        GLsizei     length;
        GLint       size;
        GLenum      utype;
        GLCommand   cmd2    = { GLCommand::GET_ACTIVE_UNIFORM, { progUid, u, uint64(sizeof(name)-1), uint64(&length), uint64(&size), uint64(&utype), uint64(name) } };
        
        ExecGL( &cmd2, 1 );
        
        for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
        {
            char    n[16];   sprintf( n, "%s_Buffer%u[0]", (type == PROG_VERTEX)?"VP":"FP", i );
            
            if( !strcmp( name, n ) )
            {            
                int         loc;
                GLCommand   cmd3 = { GLCommand::GET_UNIFORM_LOCATION, { progUid, uint64(name) } };

                ExecGL( &cmd3, 1 );
                loc = cmd3.result;

                if( loc != -1 )
                {
                    cbuf[i].location = loc;
                    cbuf[i].count    = size;
                    break;
                }
            }
        }
    }

/*
    GLint   cnt = 0;
    
    glGetProgramiv( progUid, GL_ACTIVE_UNIFORMS, &cnt );

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }
    
    for( unsigned u=0; u!=cnt; ++u )
    {    
        char    name[64];
        GLsizei length;
        GLint   size;
        GLenum  utype;
        
        glGetActiveUniform( progUid, u, sizeof(name)-1, &length, &size, &utype, name );
        DVASSERT(glGetError() == GL_NO_ERROR);
        
        for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
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
*/    
#endif // DV_USE_UNIFORMBUFFER_OBJECT


    // get texture location
    {
        char        tname[countof(texunitLoc)][16];
        GLCommand   cmd[countof(texunitLoc)];

        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            char    name[16];   

            Snprinf( name, countof(name), "Texture%u", i );
            cmd[i].func   = GLCommand::GET_UNIFORM_LOCATION;
            cmd[i].arg[0] = progUid;
            cmd[i].arg[1] = uint64(tname[i]);
        }

        ExecGL( cmd, countof(cmd) );
        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            int loc = cmd[i].result;
            
            texunitLoc[i] = (loc != -1)  ? loc  : InvalidIndex;
        }
    }
/*
    for( unsigned i=0; i!=countof(texunitLoc); ++i )
    {
        char    name[16];   Snprinf( name, countof(name), "Texture%u", i );
        int     loc = glGetUniformLocation( progUid, name );

        texunitLoc[i] = (loc != -1)  ? loc  : InvalidIndex;
    }
*/
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
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].location != InvalidIndex )
    {
        handle = ConstBufGLES2Pool::Alloc();

        ConstBuf*   cb = ConstBufGLES2Pool::Get( handle );

        if( !cb->Construct( cbuf[bufIndex].location, cbuf[bufIndex].count ) )
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
ProgGLES2::ConstBuf::Construct( unsigned loc, unsigned cnt )
{
    bool success = true;

    location    = loc;
    count       = cnt;
//    _data       = (float*)(VidMem()->alloc_aligned( cnt*4*sizeof(float), 16 ));
    data        = (float*)(malloc( cnt*4*sizeof(float) ));
    isDirty     = true;
    isInstanced = false;

    #if DV_USE_UNIFORMBUFFER_OBJECT
    GLuint  b = 0;

    glGenBuffers( 1, &b );
    glBindBuffer( GL_UNIFORM_BUFFER, b );
    glBufferData( GL_UNIFORM_BUFFER, cnt*4*sizeof(float), data, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );
    
    if( glGetError() == GL_NO_ERROR )
    {
        ubo     = b;
        success = true;
    }
    else
    {
        success = false;
    }
    #endif

    return success;
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::Destroy()
{
    if( data )
    {
//        VidMem()->free( _data );
        free( data );
        data     = 0;
        location = -1;
        count    = 0;
    }
}


//------------------------------------------------------------------------------

unsigned    
ProgGLES2::ConstBuf::ConstCount() const
{
    return count;
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::SetConst( unsigned const_i, unsigned const_count, const float* cdata )
{
    bool    success = false;

    if( const_i + const_count <= count )
    {
        memcpy( data + const_i*4, cdata, const_count*4*sizeof(float) );
        isDirty = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void*
ProgGLES2::ConstBuf::Instance() const
{
    if( isDirty )
    {
        float*  old_data = data;
        float*  new_data = DefaultConstRingBuffer.Alloc( count*4*sizeof(float) );

        memcpy( new_data, old_data, 4*count*sizeof(float) );

        if( !isInstanced )
        {
            free( data );
        }

        data        = new_data;
        isDirty     = true;
        isInstanced = true;

        return old_data;
    }
    else
    {
        return data;
    }
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::SetToRHI( void* instData ) const
{
    #if DV_USE_UNIFORMBUFFER_OBJECT
//    if( instData != data )
    {
        glBindBuffer( GL_UNIFORM_BUFFER, ubo );
        void* buf = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
        memcpy( buf, (GLfloat*)instData, count*4*sizeof(float) );
        glUnmapBuffer( GL_UNIFORM_BUFFER );

        GL_CALL(glBindBufferBase( GL_UNIFORM_BUFFER, location, ubo ));
    }
    #else
    if( instData != data )
    {
        GL_CALL(glUniform4fv( location, count, (GLfloat*)instData ));
        isDirty = false;
    }
    #endif
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

    return self->ConstCount();
}


//------------------------------------------------------------------------------

bool
SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->SetConst( const_i, const_count, data );
}

}


//------------------------------------------------------------------------------

namespace ConstBufferGLES2
{

void
InitializeRingBuffer( uint32 size )
{
    DefaultConstRingBuffer.Initialize( size );
}

void
SetToRHI( const Handle cb, void* instData )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    self->SetToRHI( instData );
}

void*
Instance( Handle cb )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->Instance();
}

}


//==============================================================================
} // namespace rhi

