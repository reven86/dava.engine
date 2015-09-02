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

struct ID3D11DeviceContext;

namespace rhi
{

void        dx11_Initialize( const InitParam& param );


namespace VertexBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb, unsigned stream_i, unsigned offset, unsigned stride, ID3D11DeviceContext* context );
}

namespace IndexBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb, unsigned offset, ID3D11DeviceContext* context );
}

namespace QueryBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );

void        BeginQuery( Handle buf, uint32 objectIndex, ID3D11DeviceContext* context );
void        EndQuery( Handle buf, uint32 objectIndex, ID3D11DeviceContext* context );
}


namespace PipelineStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
unsigned    VertexLayoutStride( Handle ps );
void        SetToRHI( Handle ps, uint32 layoutUID, ID3D11DeviceContext* context );
}

namespace ConstBufferDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        InitializeRingBuffer( uint32 size );
void        SetToRHI( Handle cb, const void* inst_data, ID3D11DeviceContext* context );
const void* InstData( Handle cb );
void        InvalidateAllConstBufferInstances();
}

namespace TextureDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHIFragment( Handle tex, unsigned unitIndex, ID3D11DeviceContext* context );
void        SetToRHIVertex( Handle tex, unsigned unitIndex, ID3D11DeviceContext* context );
void        SetRenderTarget( Handle color, Handle depthstencil, ID3D11DeviceContext* context );
Size2i      Size( Handle tex );
}


namespace DepthStencilStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state, ID3D11DeviceContext* context );
}

namespace SamplerStateDX11
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state, ID3D11DeviceContext* context );
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
        NOP                 = 0,
        
        MAP                 = 1,
        UNMAP               = 2,
        UPDATE_SUBRESOURCE  = 3
    };

    Func    func;
    uint64  arg[12];
    long    retval;
};

void     ExecDX11( DX11Command* cmd, uint32 cmdCount, bool force_immediate=false );


//==============================================================================
}
#endif // __RHI_DX11_H__

