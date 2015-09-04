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
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_metal.h"


namespace rhi
{

struct
VertexBufferMetal_t
{
                    VertexBufferMetal_t()
                      : size(0),
                        data(0),
                        uid(nil)
                    {}
                    ~VertexBufferMetal_t()
                    {}


    uint32          size;
    void*           data;
    id<MTLBuffer>   uid;
};

typedef ResourcePool<VertexBufferMetal_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,false> VertexBufferMetalPool;
RHI_IMPL_POOL(VertexBufferMetal_t,RESOURCE_VERTEX_BUFFER,VertexBuffer::Descriptor,false);


//==============================================================================


static Handle
metal_VertexBuffer_Create( const VertexBuffer::Descriptor& desc )
{
    Handle          handle  = InvalidHandle;
    id<MTLBuffer>   uid     = [_Metal_Device newBufferWithLength:desc.size options:MTLResourceOptionCPUCacheModeDefault];

    if( uid )
    {
        handle = VertexBufferMetalPool::Alloc();
        VertexBufferMetal_t*    vb = VertexBufferMetalPool::Get( handle );

        vb->data   = [uid contents];
        vb->size   = desc.size;
        vb->uid    = uid;

    }

    return handle;
}


//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Delete( Handle vb )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    if( self )
    {
        VertexBufferMetalPool::Free( vb );
    }
}


//------------------------------------------------------------------------------
    
static bool
metal_VertexBuffer_Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    bool                    success = false;
    VertexBufferMetal_t*    self    = VertexBufferMetalPool::Get( vb );

    if( offset+size <= self->size )
    {
        memcpy( ((uint8*)self->data)+offset, data, size );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

static void*
metal_VertexBuffer_Map( Handle vb, uint32 offset, uint32 size )
{
    VertexBufferMetal_t*    self = VertexBufferMetalPool::Get( vb );

    DVASSERT(self->data);
    
    return (offset+size <= self->size)  ? ((uint8*)self->data)+offset  : 0;
}


//------------------------------------------------------------------------------

static void
metal_VertexBuffer_Unmap( Handle vb )
{
    // do nothing
}





namespace VertexBufferMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_VertexBuffer_Create  = &metal_VertexBuffer_Create;
    dispatch->impl_VertexBuffer_Delete  = &metal_VertexBuffer_Delete;
    dispatch->impl_VertexBuffer_Update  = &metal_VertexBuffer_Update;
    dispatch->impl_VertexBuffer_Map     = &metal_VertexBuffer_Map;
    dispatch->impl_VertexBuffer_Unmap   = &metal_VertexBuffer_Unmap;
}

id<MTLBuffer> 
GetBuffer( Handle vb )
{
    VertexBufferMetal_t* self = VertexBufferMetalPool::Get( vb );

    return (self)  ? self->uid  : nil;
}

}


} // namespace rhi
