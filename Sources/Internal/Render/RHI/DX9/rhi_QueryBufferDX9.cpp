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
QueryBufferDX9_t
{
public:

                                    QueryBufferDX9_t();
                                    ~QueryBufferDX9_t();

    std::vector<IDirect3DQuery9*>   query;
};

typedef ResourcePool<QueryBufferDX9_t,RESOURCE_QUERY_BUFFER,QueryBuffer::Descriptor,false>    QueryBufferDX9Pool;
RHI_IMPL_POOL(QueryBufferDX9_t,RESOURCE_QUERY_BUFFER,QueryBuffer::Descriptor,false);


//==============================================================================


QueryBufferDX9_t::QueryBufferDX9_t()
{
}


//------------------------------------------------------------------------------

QueryBufferDX9_t::~QueryBufferDX9_t()
{
}

static Handle
dx9_QueryBuffer_Create( uint32 maxObjectCount )
{
    Handle              handle = QueryBufferDX9Pool::Alloc();
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf )
    {
        buf->query.resize( maxObjectCount );
        memset( &(buf->query[0]), 0, sizeof(buf->query[0])*buf->query.size() );
    }

    return handle;
}

static void
dx9_QueryBuffer_Delete( Handle handle )
{
    QueryBufferDX9_t*   buf = QueryBufferDX9Pool::Get( handle );

    if( buf )
    {
        std::vector<DX9Command> cmd;

        for( std::vector<IDirect3DQuery9*>::iterator q=buf->query.begin(),q_end=buf->query.end(); q!=q_end; ++q )
        {
            DX9Command  c  = { DX9Command::RELEASE, { uint64_t(static_cast<IUnknown*>(*q)) } };
            
            cmd.push_back( c );
        }

        ExecDX9( &cmd[0], cmd.size() );
        buf->query.clear();
    }

    QueryBufferDX9Pool::Free( handle );
}

static void
dx9_QueryBuffer_Reset( Handle handle )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf )
    {
    }
}

static bool
dx9_QueryBuffer_IsReady( Handle handle, uint32 objectIndex )
{
    bool                ready = false;
    QueryBufferDX9_t*   buf   = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        if( iq )
        {
            DWORD       val;
            DX9Command  cmd = { DX9Command::GET_QUERY_DATA, { uint64_t(iq), uint64_t(&val), sizeof(val), 0 } }; // DO NOT flush
            
            ExecDX9( &cmd, 1 );

            if( SUCCEEDED(cmd.retval) )
            {
                ready = cmd.retval == S_OK;
            }
        }
    }

    return ready;
}

static int
dx9_QueryBuffer_Value( Handle handle, uint32 objectIndex )
{
    int                 value = 0;
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        if( iq )
        {
            DWORD       val = 0;
            DX9Command  cmd = { DX9Command::GET_QUERY_DATA, { uint64_t(iq), uint64_t(&val), sizeof(val), 0 } }; // DO NOT flush

            ExecDX9( &cmd, 1 );

            if( cmd.retval == S_OK )
            {
                value = val;
            }
        }
    }

    return value;
}


namespace QueryBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_QueryBuffer_Create   = &dx9_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset    = &dx9_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete   = &dx9_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady  = &dx9_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value    = &dx9_QueryBuffer_Value;
}

void
BeginQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];

        if( !iq )
        {
            HRESULT hr = _D3D9_Device->CreateQuery( D3DQUERYTYPE_OCCLUSION, &iq );
            
            if( SUCCEEDED(hr) )
            {
                buf->query[objectIndex] = iq;
            }
            else
            {
                iq = nullptr;
            }
        }

        if( iq )
        {
            iq->Issue( D3DISSUE_BEGIN );
        }
    }
}


void
EndQuery( Handle handle, uint32 objectIndex )
{
    QueryBufferDX9_t*   buf    = QueryBufferDX9Pool::Get( handle );

    if( buf  &&  objectIndex < buf->query.size() )
    {
        IDirect3DQuery9*    iq = buf->query[objectIndex];
        
        DVASSERT(iq);
        iq->Issue( D3DISSUE_END );
    }
}

}


//==============================================================================
} // namespace rhi

