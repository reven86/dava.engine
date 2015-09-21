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
//==============================================================================

struct
IndexBufferGLES2_t
  : public ResourceImpl<IndexBufferGLES2_t,IndexBuffer::Descriptor>
{
public:
                IndexBufferGLES2_t()
                  : size(0),
                    data(nullptr),
                    uid(0),
                    is_32bit(false),
                    isMapped(false)
                {}
    
    bool        Create( const IndexBuffer::Descriptor& desc, bool force_immediate=false );
    void        Destroy( bool force_immediate=false );

    unsigned    size;
    void*       data;
    unsigned    uid;
    unsigned    is_32bit:1;
    unsigned    isMapped:1;
};
RHI_IMPL_RESOURCE(IndexBufferGLES2_t,IndexBuffer::Descriptor);

typedef ResourcePool<IndexBufferGLES2_t,RESOURCE_INDEX_BUFFER,IndexBuffer::Descriptor,true>   IndexBufferGLES2Pool;
RHI_IMPL_POOL(IndexBufferGLES2_t,RESOURCE_INDEX_BUFFER,IndexBuffer::Descriptor,true);


//------------------------------------------------------------------------------

bool
IndexBufferGLES2_t::Create( const IndexBuffer::Descriptor& desc, bool force_immediate )
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
            GLCommand   cmd2[] =
            {
                { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, b } },
                { GLCommand::BUFFER_DATA, { GL_ELEMENT_ARRAY_BUFFER, desc.size, (uint64)(desc.initialData), GL_STATIC_DRAW } },
                { GLCommand::RESTORE_INDEX_BUFFER, {} }
            };
            
            if( !desc.initialData )
                cmd2[1].func = GLCommand::NOP;

            ExecGL( cmd2, countof(cmd2), force_immediate );

            if( cmd2[0].status == GL_NO_ERROR )
            {
                void*   d = malloc( desc.size );
                
                if( d )
                {
                    data     = d;
                    size     = desc.size;
                    uid      = b;
                    is_32bit = desc.indexSize == INDEX_SIZE_32BIT;
                    isMapped = false;
                    
                    success = true;
                }
            }
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------

void
IndexBufferGLES2_t::Destroy( bool force_immediate )
{
    if( data )
    {
        GLCommand   cmd = { GLCommand::DELETE_BUFFERS, { 1, (uint64)(&uid) } };
        ExecGL( &cmd, 1, force_immediate );
        
        ::free( data );

        data = nullptr;
        size = 0;
        uid  = 0;
    }
}


//==============================================================================

static Handle
gles2_IndexBuffer_Create( const IndexBuffer::Descriptor& desc )
{
    Handle              handle = IndexBufferGLES2Pool::Alloc();
    IndexBufferGLES2_t* ib     = IndexBufferGLES2Pool::Get( handle );

    if( ib->Create( desc ) )
    {
        ib->UpdateCreationDesc( desc );
    }
    else
    {
        IndexBufferGLES2Pool::Free( handle );
        handle = InvalidHandle;
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
        self->MarkRestored();
        self->Destroy();
        IndexBufferGLES2Pool::Free( ib );
    }
}


//------------------------------------------------------------------------------
    
static bool
gles2_IndexBuffer_Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferGLES2_t* self    = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->isMapped);

    if( offset+size <= self->size )
    {
        GLCommand   cmd[] = 
        {
            { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, self->uid } },
            { GLCommand::BUFFER_DATA, { GL_ELEMENT_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
            { GLCommand::RESTORE_INDEX_BUFFER, {} }
        };

        memcpy( ((uint8*)self->data)+offset, data, size );
        ExecGL( cmd, countof(cmd) );
        success = cmd[1].status == GL_NO_ERROR;
        self->MarkRestored();
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
gles2_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->isMapped);
    DVASSERT(self->data);
    
    self->isMapped = true;

    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

static void
gles2_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(self->isMapped);

    GLCommand   cmd[] = 
    {
        { GLCommand::BIND_BUFFER, { GL_ELEMENT_ARRAY_BUFFER, self->uid } },
        { GLCommand::BUFFER_DATA, { GL_ELEMENT_ARRAY_BUFFER, self->size, (uint64)(self->data), GL_STATIC_DRAW } },
        { GLCommand::RESTORE_INDEX_BUFFER, {} }
    };

    ExecGL( cmd, countof(cmd) );
    
    self->isMapped = false;
    self->MarkRestored();
}


//------------------------------------------------------------------------------

static bool
gles2_IndexBuffer_NeedRestore( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );
    
    return self->NeedRestore();
}


//------------------------------------------------------------------------------

namespace IndexBufferGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create       = &gles2_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete       = &gles2_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update       = &gles2_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map          = &gles2_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap        = &gles2_IndexBuffer_Unmap;
    dispatch->impl_IndexBuffer_NeedRestore  = &gles2_IndexBuffer_NeedRestore;
}

IndexSize 
SetToRHI( Handle ib )
{
    IndexBufferGLES2_t* self = IndexBufferGLES2Pool::Get( ib );

    DVASSERT(!self->isMapped);
Trace("set-ib %p  sz= %u\n",self->data,self->size);
    GL_CALL(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->uid ));
    _GLES2_LastSetIB = self->uid;

    return (self->is_32bit)  ? INDEX_SIZE_32BIT  : INDEX_SIZE_16BIT;
}

void
ReCreateAll()
{
    IndexBufferGLES2Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return IndexBufferGLES2_t::NeedRestoreCount();
}


} // namespace IndexBufferGLES

//==============================================================================
} // namespace rhi

