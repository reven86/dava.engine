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


#ifndef __RHI_DX11_H__
#define __RHI_DX11_H__

#include "../rhi_Public.h"
#include "../Common/rhi_Private.h"
#include "../Common/rhi_Impl.h"


namespace rhi
{

void        dx11_Initialize( const InitParam& param );


namespace VertexBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb, unsigned stream_i, unsigned offset, unsigned stride );
}

namespace IndexBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb, unsigned offset );
}

namespace QueryBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );

void        BeginQuery( Handle buf, uint32 objectIndex );
void        EndQuery( Handle buf, uint32 objectIndex );
}


namespace PipelineStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
unsigned    VertexLayoutStride( Handle ps );
void        SetToRHI( Handle ps, uint32 layoutUID );
}

namespace ConstBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        InitializeRingBuffer( uint32 size );
void        SetToRHI( Handle cb, const void* inst_data );
const void* InstData( Handle cb );
void        InvalidateAllConstBufferInstances();
}

namespace TextureDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle tex, unsigned unitIndex );
void        SetAsRenderTarget( Handle tex );
void        SetAsDepthStencil( Handle tex );
}


namespace DepthStencilStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state );
}

namespace SamplerStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state );
}


namespace RenderPassDX11
{
void        SetupDispatch( Dispatch* dispatch );
}

namespace CommandBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
}

struct
DX11Command
{
    enum 
    Func
    {
        NOP                             = 0,
        
        CREATE_BUFFER                   = 11,
        CREATE_TEXTURE2D                = 12,
        CREATE_SAMPLER                  = 13,
        CREATE_DEPTHSTENCIL_STATE       = 14,
        CREATE_SHADEER_RESOURCE_VIEW    = 19,

        MAP_RESOURCE                    = 51,
        UNMAP_RESOURCE                  = 52,
        UPDATE_RESOURCE                 = 53,
        
        QUERY_INTERFACE                 = 101,
        RELEASE                         = 102
    };

    Func    func;
    uint64  arg[12];
    long    retval;
};

void     ExecDX11( DX11Command* cmd, uint32 cmdCount, bool force_immediate=false );


//==============================================================================
}
#endif // __RHI_DX11_H__

