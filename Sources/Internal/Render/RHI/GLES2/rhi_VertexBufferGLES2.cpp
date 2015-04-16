

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{

struct
VertexBufferGLES2_t
{
                VertexBufferGLES2_t()
                  : size(0),
                    data(nullptr),
                    uid(0),
                    mapped(false)
                {}
                ~VertexBufferGLES2_t()
                {}


    uint32      size;
    void*       data;
    uint32      uid;
    uint32      mapped:1;
};

typedef Pool<VertexBufferGLES2_t,RESOURCE_VERTEX_BUFFER>   VertexBufferGLES2Pool;
RHI_IMPL_POOL(VertexBufferGLES2_t,RESOURCE_VERTEX_BUFFER);


//==============================================================================


static Handle
gles2_VertexBuffer_Create( uint32 size, uint32 options )
{
    Handle  handle = InvalidHandle;

    DVASSERT(size);
    if( size )
    {
        GLuint      b    = 0;
        GLCommand   cmd1 = { GLCommand::GEN_BUFFERS, {1,(uint64)(&b)} };
        
        ExecGL( &cmd1, 1 );
        if( cmd1.status == GL_NO_ERROR )
        {
            GLCommand   cmd2 = { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, b } };

            ExecGL( &cmd2, 1 );
            if( cmd2.status == GL_NO_ERROR )
            {
                void*   data = malloc( size );

                if( data )
                {
                    handle = VertexBufferGLES2Pool::Alloc();
                    VertexBufferGLES2_t*    vb = VertexBufferGLES2Pool::Get( handle );

                    vb->data   = data;
                    vb->size   = size;
                    vb->uid    = b;
                    vb->mapped = false;
                }
            }
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void            
gles2_VertexBuffer_Delete( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    if( self )
    {
        if( self->data )
        {
            GLCommand   cmd = { GLCommand::DELETE_BUFFERS, { 1, (uint64)(&self->uid) } };
            ExecGL( &cmd, 1 );

            free( self->data );

            self->data = nullptr;
            self->size = 0;
            self->uid  = 0;
        }
    }
}


//------------------------------------------------------------------------------
    
bool
gles2_VertexBuffer_Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    bool                    success = false;
    VertexBufferGLES2_t*    self    = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);

    if( offset+size <= self->size )
    {
        GLCommand   cmd[] = 
        {
            { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, self->uid } },
            { GLCommand::BUFFER_DATA, { GL_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
            { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, 0 } }
        };

        memcpy( ((uint8*)self->data)+offset, data, size );
        ExecGL( cmd, countof(cmd) );
        success = cmd[1].status == GL_NO_ERROR;
    }

    return success;
}


//------------------------------------------------------------------------------

void*
gles2_VertexBuffer_Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);
    DVASSERT(self->data);
    
    self->mapped = true;

    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

void
gles2_VertexBuffer_Unmap( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(self->mapped);

    GLCommand   cmd[] = 
    {
        { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, self->uid } },
        { GLCommand::BUFFER_DATA, { GL_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
        { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, 0 } }
    };

    ExecGL( cmd, countof(cmd) );
    self->mapped = false;
}



namespace VertexBufferGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create  = &gles2_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete  = &gles2_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update  = &gles2_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map     = &gles2_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap   = &gles2_VertexBuffer_Unmap;
}

void
SetToRHI( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);
    GL_CALL(glBindBuffer( GL_ARRAY_BUFFER, self->uid ));
}

}


} // namespace rhi
