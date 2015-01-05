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


#ifndef __RHI_BASE_H__
#define __RHI_BASE_H__

    #include "rhi_Type.h"


namespace rhi
{

Api     HostApi();

void    Initialize();
void    Uninitialize();

void    Present();


////////////////////////////////////////////////////////////////////////////////
// vertex-buffer

namespace VertexBuffer
{

Handle  Create( uint32 size, uint32 options=0 );
void    Delete( Handle vb );

bool    Update( Handle vb, const void* data, uint32 offset=0, uint32 size=0 );

void*   Map( Handle vb, uint32 offset, uint32 size );
void    Unmap( Handle vb );

} // namespace VertexBuffer


////////////////////////////////////////////////////////////////////////////////
// index-buffer

namespace IndexBuffer
{

Handle  Create( uint32 size, uint32 options=0 );
void    Delete( Handle ib );

bool    Update( Handle ib, const void* data, uint32 offset=0, uint32 size=0 );

void*   Map( Handle ib, uint32 offset, uint32 size );
void    Unmap( Handle ib );

} // namespace IndexBuffer


////////////////////////////////////////////////////////////////////////////////
// pipeline-state

namespace PipelineState
{

struct
Descriptor
{
    VertexLayout    vertexLayout;
    DAVA::FastName  vprogUid;
    DAVA::FastName  fprogUid;
    BlendState      blending;
};

Handle  Create( const Descriptor& desc );
void    Delete( Handle ps );
Handle  CreateVProgConstBuffer( Handle ps, uint32 bufIndex );
Handle  CreateFProgConstBuffer( Handle ps, uint32 bufIndex );

uint32  VertexConstBufferCount( Handle ps );
uint32  VertexConstCount( Handle ps, uint32 bufIndex );
bool    GetVertexConstInfo( Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info );

uint32  FragmentConstBufferCount( Handle ps );
uint32  FragmentConstCount( Handle ps, uint32 bufIndex );
bool    GetFragmentConstInfo( Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info );

} // namespace PipelineState

namespace ConstBuffer
{

uint32  ConstCount( Handle cb );
bool    SetConst( Handle cb, uint32 constIndex, uint32 constCount, const float* data );

} // namespace ConstBuffer


namespace CommandBuffer
{

Handle  Default();

void    Begin( Handle cmdBuf );
void    End( Handle cmdBuf );
void    Clear( Handle cmdBuf );

void    SetPipelineState( Handle cmdBuf, Handle ps );

void    SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex=0 );
void    SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer );
void    SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex );

void    SetIndices( Handle cmdBuf, Handle ib );

void    SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buf );
void    SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex );

void    SetDepthStencilState( Handle cmdBuf, const DepthStencilState& bs );
void    SetSamplerState( Handle cmdBuf, const SamplerState& ss );

void    DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count );
void    DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count );


} // namespace CommandBuffer


} // namespace rhi



#define DV_USE_UNIFORMBUFFER_OBJECT 0


#endif // __RHI_BASE_H__

