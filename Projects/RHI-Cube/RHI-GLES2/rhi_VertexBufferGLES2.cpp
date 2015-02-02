

    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
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
                    data(0),
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


namespace VertexBuffer
{
//------------------------------------------------------------------------------

Handle
Create( uint32 size, uint32 options )
{
    Handle  handle = InvalidHandle;

    DVASSERT(size);
    if( size )
    {
        GLuint  b = 0;

        glGenBuffers( 1, &b );
        glBindBuffer( GL_ARRAY_BUFFER, b );

        if( glGetError() == GL_NO_ERROR )
        {
//            void*   data = VidMem()->alloc_aligned( size, 16 );
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

    return handle;
}


//------------------------------------------------------------------------------

void            
Delete( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    if( self )
    {
        if( self->data )
        {
            GL_CALL(glDeleteBuffers( 1, &self->uid ));
//            VidMem()->free( self->_data );
            free( self->data );

            self->data = 0;
            self->size = 0;
            self->uid  = 0;
        }

        delete self;
    }
}


//------------------------------------------------------------------------------
    
bool
Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    bool                    success = false;
    VertexBufferGLES2_t*    self    = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);

    if( offset+size <= self->size )
    {
        memcpy( ((uint8*)self->data)+offset, data, size );
        glBindBuffer( GL_ARRAY_BUFFER, self->uid );
        glBufferData( GL_ARRAY_BUFFER, self->size, self->data, GL_STATIC_DRAW );
        success = glGetError() == GL_NO_ERROR;
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    return success;
}


//------------------------------------------------------------------------------

void*
Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);
    DVASSERT(self->data);
    
    self->mapped = true;

    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

void
Unmap( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(self->mapped);
    
    GL_CALL(glBindBuffer( GL_ARRAY_BUFFER, self->uid ));
    GL_CALL(glBufferData( GL_ARRAY_BUFFER, self->size, self->data, GL_STATIC_DRAW ));
    GL_CALL(glBindBuffer( GL_ARRAY_BUFFER, 0 ));
    self->mapped = false;
}


} // namespace VertexBuffer





namespace VertexBufferGLES2
{
void
SetToRHI( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->mapped);
    GL_CALL(glBindBuffer( GL_ARRAY_BUFFER, self->uid ));
}
}


} // namespace rhi
