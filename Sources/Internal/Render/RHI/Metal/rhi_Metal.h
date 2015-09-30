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


#ifndef __RHI_METAL_H__
#define __RHI_METAL_H__

#include "../Common/rhi_Private.h"
#include "../Common/rhi_Impl.h"
#if defined __OBJC__
#include <Metal/Metal.h>
#endif

namespace rhi
{

struct InitParam;

void        metal_Initialize( const InitParam& param );


#if defined __OBJC__

namespace VertexBufferMetal
{
id<MTLBuffer>   GetBuffer( Handle ib );
}

namespace IndexBufferMetal
{
id<MTLBuffer>   GetBuffer( Handle ib );
MTLIndexType    GetType( Handle ib );
}

namespace QueryBufferMetal
{
id<MTLBuffer> GetBuffer( Handle qb );
}


namespace TextureMetal
{
void    SetToRHIFragment( Handle tex, unsigned unitIndex, id<MTLRenderCommandEncoder> ce );
void    SetToRHIVertex( Handle tex, unsigned unitIndex, id<MTLRenderCommandEncoder> ce );
void    SetAsRenderTarget( Handle tex, MTLRenderPassDescriptor* desc );
void    SetAsDepthStencil( Handle tex, MTLRenderPassDescriptor* desc );
}


namespace PipelineStateMetal
{
uint32    SetToRHI( Handle ps, uint32 layoutUID, bool ds_used, id<MTLRenderCommandEncoder> ce );
}

namespace DepthStencilStateMetal
{
void    SetToRHI( Handle ds, id<MTLRenderCommandEncoder> ce );
}

namespace SamplerStateMetal
{
    void    SetToRHI( Handle ss, id<MTLRenderCommandEncoder> ce );
}

namespace ConstBufferMetal
{
void    InitializeRingBuffer( uint32 size );
void    InvalidateAllInstances();

void    SetToRHI( Handle buf, unsigned bufIndex, id<MTLRenderCommandEncoder> ce );
}


#endif

    
namespace VertexBufferMetal
{
    void        Init( uint32 maxCount );
    void        SetupDispatch( Dispatch* dispatch );
}
namespace IndexBufferMetal
{
    void        Init( uint32 maxCount );
    void        SetupDispatch( Dispatch* dispatch );
}
namespace QueryBufferMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}
namespace TextureMetal
{
    void        Init( uint32 maxCount );
    void        SetupDispatch( Dispatch* dispatch );
}
namespace SamplerStateMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}
namespace PipelineStateMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}
namespace DepthStencilStateMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}
namespace ConstBufferMetal
{
    void        Init( uint32 maxCount );
    void        SetupDispatch( Dispatch* dispatch );
}
namespace RenderPassMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}

namespace CommandBufferMetal
{
    void        SetupDispatch( Dispatch* dispatch );
}

const unsigned  QueryBUfferElemeentAlign = 8;
    

//==============================================================================
}
#endif // __RHI_METAL_H__

