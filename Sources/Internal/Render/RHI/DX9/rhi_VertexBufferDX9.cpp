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
    #include "rhi_DX9.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx9.h"


namespace rhi
{
//==============================================================================


class
VertexBufferDX9_t
  : public ResourceImpl<VertexBufferDX9_t,VertexBuffer::Descriptor>
{
public:
                            VertexBufferDX9_t();
                            ~VertexBufferDX9_t();

    bool                    Create( const VertexBuffer::Descriptor& desc, bool force_immediate=false );
    void                    Destroy( bool force_immediate=false);

    unsigned                size;
    IDirect3DVertexBuffer9* buffer;
    unsigned                isMapped:1;

    IDirect3DVertexBuffer9* prevBuffer;
};
RHI_IMPL_RESOURCE(VertexBufferDX9_t,VertexBuffer::Descriptor);


typedef ResourcePool<VertexBufferDX9_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,true>   VertexBufferDX9Pool;

RHI_IMPL_POOL(VertexBufferDX9_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,true);


VertexBufferDX9_t::VertexBufferDX9_t()
  : size(0),
    buffer(nullptr),
    isMapped(false),
    prevBuffer(nullptr)
{
}


//------------------------------------------------------------------------------

VertexBufferDX9_t::~VertexBufferDX9_t()
{
}


//------------------------------------------------------------------------------

bool
VertexBufferDX9_t::Create( const VertexBuffer::Descriptor& desc, bool force_immediate )
{
    DVASSERT(desc.size);
    bool    success = false;

    if( desc.size )
    {
        DWORD   usage = D3DUSAGE_WRITEONLY;

        switch( desc.usage )
        {
            case USAGE_DEFAULT      : usage = D3DUSAGE_WRITEONLY; break;
            case USAGE_STATICDRAW   : usage = D3DUSAGE_WRITEONLY; break;
            case USAGE_DYNAMICDRAW  : usage = D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC; break;
        }

        DX9Command  cmd[] = { { DX9Command::CREATE_VERTEX_BUFFER, { desc.size, usage, 0, D3DPOOL_DEFAULT, uint64_t(&buffer), NULL } } };
        
        ExecDX9( cmd, countof(cmd), force_immediate );

        if( SUCCEEDED(cmd[0].retval) )
        {
            size     = desc.size;
            isMapped = false;

            success  = true;
        }
        else
        {
            Logger::Error( "FAILED to create vertex-buffer:\n%s\n", D3D9ErrorText(cmd[0].retval) );
        }
    }

    return success;
}


//------------------------------------------------------------------------------

void
VertexBufferDX9_t::Destroy( bool force_immediate )
{
    if( buffer )
    {
        DX9Command  cmd[] = { DX9Command::RELEASE, { uint64_t(static_cast<IUnknown*>(buffer)) } };

        prevBuffer = buffer;
        ExecDX9( cmd, countof(cmd), force_immediate );
        buffer = nullptr;
    }

    size = 0;
}


//==============================================================================


//------------------------------------------------------------------------------

static Handle
dx9_VertexBuffer_Create( const VertexBuffer::Descriptor& desc )
{
    Handle              handle = VertexBufferDX9Pool::Alloc();
    VertexBufferDX9_t*  vb     = VertexBufferDX9Pool::Get( handle );
    
    if( vb->Create( desc ) )
    {
        vb->UpdateCreationDesc( desc );
    }
    else
    {
        VertexBufferDX9Pool::Free( handle );
        handle = InvalidHandle;
    }
    
    return handle;
}


//------------------------------------------------------------------------------

static void            
dx9_VertexBuffer_Delete( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );

    if( self )
    {
        self->MarkRestored();
        self->Destroy();
        VertexBufferDX9Pool::Free( vb );
    }
}


//------------------------------------------------------------------------------
    
static bool
dx9_VertexBuffer_Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    VertexBufferDX9_t*  self    = VertexBufferDX9Pool::Get( vb );

    DVASSERT(!self->isMapped);

    if( offset+size <= self->size )
    {
        void*       ptr  = nullptr;
        DX9Command  cmd1 = { DX9Command::LOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)), offset, size, uint64_t(&ptr), 0 } };
        
        ExecDX9( &cmd1, 1 );
        if( SUCCEEDED(cmd1.retval) )
        {
            memcpy( ptr, data, size );

            DX9Command  cmd2 = { DX9Command::UNLOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)) } };
            
            ExecDX9( &cmd2, 1 );
            success = true;
            
            self->MarkRestored();            
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
dx9_VertexBuffer_Map( Handle vb, unsigned offset, unsigned size )
{
    void*               ptr  = nullptr;
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    DX9Command          cmd  = { DX9Command::LOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)), offset, size, uint64_t(&ptr), 0 } };

    DVASSERT(!self->isMapped);
    ExecDX9( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->isMapped = true;
    }

    return ptr;
}


//------------------------------------------------------------------------------

static void
dx9_VertexBuffer_Unmap( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    DX9Command          cmd  = { DX9Command::UNLOCK_VERTEX_BUFFER, { uint64_t(&(self->buffer)) } };
    
    DVASSERT(self->isMapped);
    ExecDX9( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->isMapped = false;
        self->MarkRestored();        
    }
}


//------------------------------------------------------------------------------

static bool
dx9_VertexBuffer_NeedRestore( Handle vb )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    
    return self->NeedRestore();
}



//------------------------------------------------------------------------------

namespace VertexBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create      = &dx9_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete      = &dx9_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update      = &dx9_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map         = &dx9_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap       = &dx9_VertexBuffer_Unmap;
    dispatch->impl_VertexBuffer_NeedRestore = &dx9_VertexBuffer_NeedRestore;
}


void 
SetToRHI( Handle vb, unsigned stream_i, unsigned offset, unsigned stride  )
{
    VertexBufferDX9_t*  self = VertexBufferDX9Pool::Get( vb );
    HRESULT             hr   = _D3D9_Device->SetStreamSource( stream_i, self->buffer, offset, stride );

    DVASSERT(!self->isMapped);

    if( FAILED(hr) )    
        Logger::Error( "SetStreamSource failed:\n%s\n", D3D9ErrorText(hr) );
}

void
ReleaseAll()
{
    for( VertexBufferDX9Pool::Iterator b=VertexBufferDX9Pool::Begin(),b_end=VertexBufferDX9Pool::End(); b!=b_end; ++b )
    {
        b->Destroy( true );
    }
}

void
ReCreateAll()
{
    VertexBufferDX9Pool::ReCreateAll();
}

unsigned
NeedRestoreCount()
{
    return VertexBufferDX9_t::NeedRestoreCount();
}

void
PatchCommands( DX9Command* command, uint32 cmdCount )
{
    for( VertexBufferDX9Pool::Iterator b=VertexBufferDX9Pool::Begin(),b_end=VertexBufferDX9Pool::End(); b!=b_end; ++b )
    {
        if( b->prevBuffer )
        {
            for( DX9Command* cmd=command,*cmd_end=command+cmdCount; cmd!=cmd_end; ++cmd )
            {
                if(     cmd->func == DX9Command::LOCK_VERTEX_BUFFER 
                    &&  (IDirect3DVertexBuffer9*)(cmd->arg[0]) == b->prevBuffer
                  )
                {
                    cmd->arg[0] = uint64(b->buffer);
                }                    
            }
        }
    }
}


}


} // namespace rhi
