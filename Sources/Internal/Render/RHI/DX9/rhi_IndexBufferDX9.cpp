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
IndexBufferDX9_t
{
public:
                            IndexBufferDX9_t();
                            ~IndexBufferDX9_t();


    unsigned                size;
    IDirect3DIndexBuffer9*  buffer;
    unsigned                isMapped:1;
};

typedef ResourcePool<IndexBufferDX9_t,RESOURCE_INDEX_BUFFER>    IndexBufferDX9Pool;
RHI_IMPL_POOL(IndexBufferDX9_t,RESOURCE_INDEX_BUFFER);


//==============================================================================


IndexBufferDX9_t::IndexBufferDX9_t()
  : size(0),
    buffer(nullptr),
    isMapped(false)
{
}


//------------------------------------------------------------------------------

IndexBufferDX9_t::~IndexBufferDX9_t()
{
}


//------------------------------------------------------------------------------

static Handle
dx9_IndexBuffer_Create( const IndexBuffer::Descriptor& desc )
{
    Handle  handle = InvalidIndex;

    DVASSERT(desc.size);
    if( desc.size )
    {
        DWORD   usage = D3DUSAGE_WRITEONLY;

        switch( desc.usage )
        {
            case USAGE_DEFAULT      : usage = D3DUSAGE_WRITEONLY; break;
            case USAGE_STATICDRAW   : usage = D3DUSAGE_WRITEONLY; break;
            case USAGE_DYNAMICDRAW  : usage = D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC; break;
        }

        IDirect3DIndexBuffer9*  ib9   = nullptr;
        DX9Command              cmd[] = { { DX9Command::CREATE_INDEX_BUFFER, { desc.size, usage, D3DFMT_INDEX16, D3DPOOL_DEFAULT, uint64_t(&ib9), NULL } } };
        
        ExecDX9( cmd, countof(cmd) );

        if( SUCCEEDED(cmd[0].retval) )
        {
            handle = IndexBufferDX9Pool::Alloc();

            IndexBufferDX9_t* ib = IndexBufferDX9Pool::Get( handle );
            
            ib->size     = desc.size;
            ib->buffer   = ib9;
            ib->isMapped = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D9ErrorText(cmd[0].retval) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_IndexBuffer_Delete( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    
    if( self )
    {
        if( self->buffer )
        {
            DX9Command  cmd[] = { DX9Command::RELEASE, { uint64_t(static_cast<IUnknown*>(self->buffer)) } };

            ExecDX9( cmd, countof(cmd) );
            self->buffer = nullptr;
        }

        self->size = 0;
        
        IndexBufferDX9Pool::Free( ib );
    }    
}


//------------------------------------------------------------------------------
    
static bool
dx9_IndexBuffer_Update( Handle ib, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferDX9_t*   self    = IndexBufferDX9Pool::Get( ib );

    DVASSERT(!self->isMapped);

    if( offset+size <= self->size )
    {
        void*       ptr  = nullptr;
        DX9Command  cmd1 = { DX9Command::LOCK_INDEX_BUFFER, { uint64_t(self->buffer), offset, size, uint64_t(&ptr), 0 } };
        
        ExecDX9( &cmd1, 1 );
        if( SUCCEEDED(cmd1.retval) )
        {
            memcpy( ptr, data, size );

            DX9Command  cmd2 = { DX9Command::UNLOCK_INDEX_BUFFER, { uint64_t(self->buffer) } };
            
            ExecDX9( &cmd2, 1 );
            success = true;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
dx9_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    void*               ptr  = nullptr;
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    DX9Command          cmd  = { DX9Command::LOCK_INDEX_BUFFER, { uint64_t(self->buffer), offset, size, uint64_t(&ptr), 0 } };

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
dx9_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    DX9Command          cmd  = { DX9Command::UNLOCK_INDEX_BUFFER, { uint64_t(self->buffer) } };
    
    DVASSERT(self->isMapped);
    ExecDX9( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        self->isMapped = false;
    }
}



//------------------------------------------------------------------------------

namespace IndexBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create   = &dx9_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete   = &dx9_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update   = &dx9_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map      = &dx9_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap    = &dx9_IndexBuffer_Unmap;
}

void 
SetToRHI( Handle ib )
{
    IndexBufferDX9_t*   self = IndexBufferDX9Pool::Get( ib );
    HRESULT             hr   = _D3D9_Device->SetIndices( self->buffer );

    DVASSERT(!self->isMapped);

    if( FAILED(hr) )    
        Logger::Error( "SetIndices failed:\n%s\n", D3D9ErrorText(hr) );
}

}

//==============================================================================
} // namespace rhi

