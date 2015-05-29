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


#ifndef __RHI_PUBLIC_H__
#define __RHI_PUBLIC_H__

    #include "rhi_Type.h"


namespace rhi
{
////////////////////////////////////////////////////////////////////////////////
// base operation

struct
InitParam
{
//    uint32  width;
//    uint32  height;
    void*   context;

    void    (*makeCurrentFunc)();
    void    (*endFrameFunc)();
};

struct
ResetParam
{
//    uint32  width;
//    uint32  height;
};


void    Initialize( Api api, const InitParam& param );
void    Uninitialize();
void    Reset( const ResetParam& param );

void    Present(); // execute all submitted command-buffers & do flip/present

Api     HostApi();
bool    TextureFormatSupported( TextureFormat format );


////////////////////////////////////////////////////////////////////////////////
// resource-handle

template <ResourceType T>
class
ResourceHandle
{
public:
                        ResourceHandle()                            : handle(InvalidHandle) {}
    explicit            ResourceHandle( Handle h )                  : handle(h) {}
    bool                IsValid() const                             { return handle != InvalidHandle; }
    operator            Handle() const                              { return handle; }
    ResourceHandle<T>&  operator=( const ResourceHandle<T>& src )   { handle=src.handle; return *this; }

private:

    Handle      handle;
};


////////////////////////////////////////////////////////////////////////////////
// vertex buffer

typedef ResourceHandle<RESOURCE_VERTEX_BUFFER> HVertexBuffer;

HVertexBuffer   CreateVertexBuffer( uint32 size );
void            DeleteVertexBuffer( HVertexBuffer vb );

void*           MapVertexBuffer( HVertexBuffer vb, uint32 offset, uint32 size );
void            UnmapVertexBuffer( HVertexBuffer vb );

void            UpdateVertexBuffer( HVertexBuffer vb, const void* data, uint32 offset, uint32 size );


////////////////////////////////////////////////////////////////////////////////
// index buffer

typedef ResourceHandle<RESOURCE_INDEX_BUFFER> HIndexBuffer;

HIndexBuffer    CreateIndexBuffer( uint32 size );
void            DeleteIndexBuffer( HIndexBuffer ib );

void*           MapIndexBuffer( HIndexBuffer ib, uint32 offset, uint32 size );
void            UnmapIndexBuffer( HIndexBuffer ib );

void            UpdateIndexBuffer( HIndexBuffer ib, const void* data, uint32 offset, uint32 size );


////////////////////////////////////////////////////////////////////////////////
// render-pipeline state & const-buffers

typedef ResourceHandle<RESOURCE_PIPELINE_STATE> HPipelineState;
typedef ResourceHandle<RESOURCE_CONST_BUFFER>   HConstBuffer;


HPipelineState  AcquireRenderPipelineState( const PipelineState::Descriptor& desc );
void            ReleaseRenderPipelineState( HPipelineState rps );

HConstBuffer    CreateVertexConstBuffer( HPipelineState rps, uint32 bufIndex );
bool            CreateVertexConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf );

HConstBuffer    CreateFragmentConstBuffer( HPipelineState rps, uint32 bufIndex );
bool            CreateFragmentConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf );

bool            UpdateConstBuffer4fv( HConstBuffer constBuf, uint32 constIndex, const float* data, uint32 constCount );
bool            UpdateConstBuffer1fv( HConstBuffer constBuf, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount );
void            DeleteConstBuffer( HConstBuffer constBuf );


////////////////////////////////////////////////////////////////////////////////
// texture-set

typedef ResourceHandle<RESOURCE_TEXTURE> HTexture;
typedef ResourceHandle<RESOURCE_TEXTURE_SET> HTextureSet;

HTexture        CreateTexture( const Texture::Descriptor& desc );
void            DeleteTexture( HTexture tex );

void*           MapTexture( HTexture tex, uint32 level=0 );
void            UnmapTexture( HTexture tex );

void            UpdateTexture( HTexture tex, const void* data, uint32 level, TextureFace face=TEXTURE_FACE_LEFT );


struct
TextureSetDescriptor
{
    uint32  count;
    HTexture    texture[MAX_TEXTURE_SAMPLER_COUNT];
};

HTextureSet     AcquireTextureSet( const TextureSetDescriptor& desc );
HTextureSet     CopyTextureSet( HTextureSet ts );
void            ReleaseTextureSet( HTextureSet ts );


////////////////////////////////////////////////////////////////////////////////
//  depthstencil-state

typedef ResourceHandle<RESOURCE_DEPTHSTENCIL_STATE> HDepthStencilState;

HDepthStencilState  AcquireDepthStencilState( const DepthStencilState::Descriptor& desc );
HDepthStencilState  CopyDepthStencilState( HDepthStencilState ds );
void                ReleaseDepthStencilState( HDepthStencilState ds );


////////////////////////////////////////////////////////////////////////////////
//  sampler-state

typedef ResourceHandle<RESOURCE_SAMPLER_STATE> HSamplerState;

HSamplerState       AcquireSamplerState( const SamplerState::Descriptor& desc );
HSamplerState       CopySamplerState( HSamplerState ss );
void                ReleaseSamplerState( HSamplerState ss );



////////////////////////////////////////////////////////////////////////////////
// render-pass

typedef ResourceHandle<RESOURCE_RENDER_PASS> HRenderPass;
typedef ResourceHandle<RESOURCE_PACKET_LIST> HPacketList;

HRenderPass         AllocateRenderPass( const RenderPassConfig& passDesc, uint32 packetListCount, HPacketList* packetList );
void                BeginRenderPass( HRenderPass pass );
void                EndRenderPass( HRenderPass pass ); // no explicit render-pass 'release' needed


////////////////////////////////////////////////////////////////////////////////
// rendering

struct
Packet
{
    uint32              vertexStreamCount;
    HVertexBuffer       vertexStream[MAX_VERTEX_STREAM_COUNT];
    uint32              vertexCount;
    uint32              baseVertex;
    uint32              startIndex;
    uint32              vertexLayoutUID;
    HIndexBuffer        indexBuffer;
    HPipelineState      renderPipelineState;
    HDepthStencilState  depthStencilState;
    HSamplerState       samplerState;
    CullMode            cullMode;
    uint32              vertexConstCount;
    HConstBuffer        vertexConst[MAX_CONST_BUFFER_COUNT];
    uint32              fragmentConstCount;
    HConstBuffer        fragmentConst[MAX_CONST_BUFFER_COUNT];
    HTextureSet         fragmentTextureSet;
    HTextureSet         vertexTextureSet;
    PrimitiveType       primitiveType;
    uint32              primitiveCount;
    const char*         debugMarker;

                        Packet()
                          : vertexStreamCount(0),
                            vertexCount(0),
                            baseVertex(0),
                            startIndex(0),
                            vertexLayoutUID(VertexLayout::InvalidUID),
                            depthStencilState(InvalidHandle),
                            samplerState(InvalidHandle),
                            cullMode(CULL_CCW),
                            vertexConstCount(0),
                            fragmentConstCount(0),
                            primitiveCount(0),
                            debugMarker(nullptr)
                        {
                        }
};

void    BeginPacketList( HPacketList packetList );
void    AddPackets( HPacketList packetList, const Packet* packet, uint32 packetCount );
void    AddPacket( HPacketList packetList, const Packet& packet );
void    EndPacketList( HPacketList packetList ); // 'packetList' handle invalid after this, no explicit "release" needed



} // namespace rhi
#endif // __RHI_PUBLIC_H__

