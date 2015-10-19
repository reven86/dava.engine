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
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{

struct
VertexBufferGLES2_t
  : public ResourceImpl<VertexBufferGLES2_t,VertexBuffer::Descriptor>
{
                VertexBufferGLES2_t()
                  : size(0),
                    mappedData(nullptr),
                    uid(0),
					usage(USAGE_DEFAULT),
                    isMapped(false)
                {}
                ~VertexBufferGLES2_t()
                {}

    bool        Create( const VertexBuffer::Descriptor& desc, bool force_immediate=false );
    void        Destroy( bool force_immediate=false);


    uint32      size;
    void*       mappedData;
    uint32      uid;
    GLenum      usage;
    uint32      isMapped:1;
    uint32      updatePending:1;
};
RHI_IMPL_RESOURCE(VertexBufferGLES2_t,VertexBuffer::Descriptor);

typedef ResourcePool<VertexBufferGLES2_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,true>   VertexBufferGLES2Pool;
RHI_IMPL_POOL(VertexBufferGLES2_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,true);


//------------------------------------------------------------------------------

bool
VertexBufferGLES2_t::Create( const VertexBuffer::Descriptor& desc, bool force_immediate )
{
    bool    success = false;

    DVASSERT(desc.size);
    if( desc.size )
    {
        GLuint      b    = 0;
        GLCommand   cmd1 = { GLCommand::GEN_BUFFERS, {1,(uint64)(&b)} };
        
        ExecGL( &cmd1, 1, force_immediate );
        if( cmd1.status == GL_NO_ERROR )
        {
            switch( desc.usage )
            {
                case USAGE_DEFAULT      : usage = GL_DYNAMIC_DRAW; break;
                case USAGE_STATICDRAW   : usage = GL_STATIC_DRAW; break;
                case USAGE_DYNAMICDRAW  : usage = GL_DYNAMIC_DRAW; break;
            }

            GLCommand   cmd2[] =
            {
                { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, uint64(&b) } },
                { GLCommand::BUFFER_DATA, { GL_ARRAY_BUFFER, desc.size, (uint64)(desc.initialData), usage } },
                { GLCommand::RESTORE_VERTEX_BUFFER, {} }
            };

            if( !desc.initialData )
            {
                DVASSERT(desc.usage != USAGE_STATICDRAW);
                cmd2[1].func = GLCommand::NOP;
            }

            ExecGL( cmd2, countof(cmd2), force_immediate );
            
            if( cmd2[0].status == GL_NO_ERROR )
            {
                mappedData    = nullptr;
                size          = desc.size;
                uid           = b;
                isMapped      = false;
                updatePending = false;
                    
                success = true;
            }
        }
    }

    return success;
}


//------------------------------------------------------------------------------

void
VertexBufferGLES2_t::Destroy( bool force_immediate )
{
    GLCommand   cmd = { GLCommand::DELETE_BUFFERS, { 1, (uint64)(&uid) } };
    ExecGL( &cmd, 1, force_immediate );

    if( mappedData )
    {
        ::free( mappedData );
        mappedData = nullptr;
    }

    size = 0;
    uid  = 0;
}



//==============================================================================


static Handle
gles2_VertexBuffer_Create( const VertexBuffer::Descriptor& desc )
{
    Handle                  handle = VertexBufferGLES2Pool::Alloc();
    VertexBufferGLES2_t*    vb     = VertexBufferGLES2Pool::Get( handle );

    if( vb->Create( desc ) )
    {
        VertexBuffer::Descriptor creationDesc(desc);
        creationDesc.initialData = nullptr;
        vb->UpdateCreationDesc(creationDesc);
    }
    else
    {
        VertexBufferGLES2Pool::Free( handle );
        handle = InvalidHandle;
    }
    
    return handle;
}


//------------------------------------------------------------------------------

void            
gles2_VertexBuffer_Delete( Handle vb )
{
    if (vb != InvalidHandle)
    {
        VertexBufferGLES2_t* self = VertexBufferGLES2Pool::Get(vb);

        self->MarkRestored();
        self->Destroy();
        VertexBufferGLES2Pool::Free( vb );
    }
}


//------------------------------------------------------------------------------
    
bool
gles2_VertexBuffer_Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    bool                    success = false;
    VertexBufferGLES2_t*    self    = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(self->usage != GL_STATIC_DRAW);
    DVASSERT(!self->isMapped);

    if( offset+size <= self->size )
    {
        GLCommand   cmd[] = 
        {
            { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, uint64(&(self->uid)) } },
            { GLCommand::BUFFER_DATA, { GL_ARRAY_BUFFER, self->size, (uint64)(data), self->usage } },
            { GLCommand::RESTORE_VERTEX_BUFFER, {} }
        };

        ExecGL( cmd, countof(cmd) );
        success = cmd[1].status == GL_NO_ERROR;
        self->MarkRestored();
    }

    return success;
}


//------------------------------------------------------------------------------

void*
gles2_VertexBuffer_Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );
    void*                   data = nullptr;

    DVASSERT(self->usage != GL_STATIC_DRAW);
    DVASSERT(!self->isMapped);
    
    if( offset+size <= self->size )    
    {
        if( !self->mappedData )
            self->mappedData = ::malloc( self->size );

        self->isMapped = true;
        data = ((uint8*)self->mappedData) + offset;
    }

    return data;
}


//------------------------------------------------------------------------------

void
gles2_VertexBuffer_Unmap( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(self->usage != GL_STATIC_DRAW);
    DVASSERT(self->isMapped);

    if( self->usage == GL_DYNAMIC_DRAW )
    {
        self->isMapped      = false;
        self->updatePending = true;
    }
    else
    {
        GLCommand   cmd[] = 
        {
            { GLCommand::BIND_BUFFER, { GL_ARRAY_BUFFER, uint64(&(self->uid)) } },
            { GLCommand::BUFFER_DATA, { GL_ARRAY_BUFFER, self->size, (uint64)(self->mappedData), self->usage } },
            { GLCommand::RESTORE_VERTEX_BUFFER, {} }
        };

        ExecGL( cmd, countof(cmd) );
        self->isMapped = false;
        self->MarkRestored();

        ::free( self->mappedData );
        self->mappedData = nullptr;
    }
}


//------------------------------------------------------------------------------

static bool
gles2_VertexBuffer_NeedRestore( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );
    
    return self->NeedRestore();
}



namespace VertexBufferGLES2
{

void
Init( uint32 maxCount )
{
    VertexBufferGLES2Pool::Reserve( maxCount );
}

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create      = &gles2_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete      = &gles2_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update      = &gles2_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map         = &gles2_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap       = &gles2_VertexBuffer_Unmap;
    dispatch->impl_VertexBuffer_NeedRestore = &gles2_VertexBuffer_NeedRestore;
}

void
SetToRHI( Handle vb )
{
    VertexBufferGLES2_t*    self = VertexBufferGLES2Pool::Get( vb );

    DVASSERT(!self->isMapped);
    GL_CALL(glBindBuffer( GL_ARRAY_BUFFER, self->uid ));
    _GLES2_LastSetVB = self->uid;

    if( self->updatePending )
    {
        DVASSERT(self->mappedData);
        glBufferData( GL_ARRAY_BUFFER, self->size, self->mappedData, self->usage );
        self->updatePending = false;
    }
}

void
ReCreateAll()
{
    VertexBufferGLES2Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return VertexBufferGLES2_t::NeedRestoreCount();
}

}


} // namespace rhi
