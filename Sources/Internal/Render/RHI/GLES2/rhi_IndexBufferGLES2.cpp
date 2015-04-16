
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{
//==============================================================================

struct
IndexBufferGLES2_t
{
public:
                IndexBufferGLES2_t()
                  : size(0),
                    data(nullptr),
                    uid(0),
                    mapped(false)
                {}


    unsigned    size;
    void*       data;
    unsigned    uid;
    unsigned    mapped:1;
};

typedef Pool<IndexBufferGLES2_t,RESOURCE_INDEX_BUFFER>   IndexBufferGLES2Pool;
RHI_IMPL_POOL(IndexBufferGLES2_t,RESOURCE_INDEX_BUFFER);


//==============================================================================

static Handle
gles2_IndexBuffer_Create( uint32 size, uint32 options )
{
    Handle  handle = InvalidIndex;

    DVASSERT(size);
    if( size )
    {
        GLuint      b    = 0;
        GLCommand   cmd1 = { GLCommand::GEN_BUFFERS, {1,(uint64)(&b)} };
        
        ExecGL( &cmd1, 1 );
        if( cmd1.status == GL_NO_ERROR )
        {
            GLCommand   cmd2 = { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, b } };

            ExecGL( &cmd2, 1 );
            if( cmd2.status == GL_NO_ERROR )
            {
                void*   data = malloc( size );
                
                if( data )
                {
                    handle = IndexBufferGLES2Pool::Alloc();
                    
                    IndexBufferGLES2_t* ib = IndexBufferGLES2Pool::Get( handle );
                    
                    ib->data   = data;
                    ib->size   = size;
                    ib->uid    = b;
                    ib->mapped = false;
                }
            }
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
gles2_IndexBuffer_Delete( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );
    
    if( self )
    {
        if( self->data )
        {
            GLCommand   cmd = { GLCommand::DELETE_BUFFERS, { 1, (uint64)(&self->uid) } };
            ExecGL( &cmd, 1 );

            self->data = nullptr;
            self->size = 0;
            self->uid  = 0;
        }

        IndexBufferGLES2Pool::Free( ib );
    }

}


//------------------------------------------------------------------------------
    
static bool
gles2_IndexBuffer_Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferGLES2_t* self    = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->mapped);

    if( offset+size <= self->size )
    {
        GLCommand   cmd[] = 
        {
            { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, self->uid } },
            { GLCommand::BUFFER_DATA, { GL_ELEMENT_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
            { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, 0 } }
        };

        memcpy( ((uint8*)self->data)+offset, data, size );
        ExecGL( cmd, countof(cmd) );
        success = cmd[1].status == GL_NO_ERROR;
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
gles2_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->mapped);
    DVASSERT(self->data);
    
    self->mapped = true;

    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

static void
gles2_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(self->mapped);

    GLCommand   cmd[] = 
    {
        { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, self->uid } },
        { GLCommand::BUFFER_DATA, { GL_ELEMENT_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
        { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, 0 } }
    };

    ExecGL( cmd, countof(cmd) );
    
    self->mapped = false;
}


//------------------------------------------------------------------------------

namespace IndexBufferGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create  = &gles2_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete  = &gles2_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update  = &gles2_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map     = &gles2_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap   = &gles2_IndexBuffer_Unmap;
}

void 
SetToRHI( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->mapped);
    GL_CALL(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->uid ));
}


} // namespace IndexBufferGLES

//==============================================================================
} // namespace rhi

