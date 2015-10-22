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

namespace DAVA { class Mutex; }

namespace rhi
{
////////////////////////////////////////////////////////////////////////////////
// base operation

struct
InitParam
{
    uint32          width;
    uint32          height;
    float32         scaleX;
    float32         scaleY;
    void*           window;
    uint32          fullScreen:1;
    uint32          threadedRenderEnabled:1;
    uint32          threadedRenderFrameCount;
    DAVA::Mutex*    FrameCommandExecutionSync;

    uint32          maxIndexBufferCount;
    uint32          maxVertexBufferCount;
    uint32          maxConstBufferCount;
    uint32          maxTextureCount;
    uint32          ringBufferSize;

    void            (*acquireContextFunc)();
    void            (*releaseContextFunc)();
            
                    InitParam()
                      : width(0),
                        height(0),
                        scaleX(1.0f),
                        scaleY(1.0f),
                        window(nullptr),
                        fullScreen(false),
                        threadedRenderEnabled(false),
                        threadedRenderFrameCount(2),
                        FrameCommandExecutionSync(nullptr),
                        maxIndexBufferCount(0),
                        maxVertexBufferCount(0),
                        maxConstBufferCount(0),
                        maxTextureCount(0),
                        ringBufferSize(4*1024*1024),
                        acquireContextFunc(nullptr),
                        releaseContextFunc(nullptr)
                    {}
};

struct
ResetParam
{
    uint32  width;
    uint32  height;
    float32 scaleX;
    float32 scaleY;
    void*   window;
    uint32  fullScreen:1;

    ResetParam()
      : width(0), 
        height(0), 
        scaleX(1.0f), 
        scaleY(1.0f),
        fullScreen(false)
    	, window(nullptr)
    {}
};

struct
RenderDeviceCaps
{
    bool is32BitIndicesSupported = false;
    bool isVertexTextureUnitsSupported = false;
    bool isFramebufferFetchSupported = false;
    
    bool isUpperLeftRTOrigin = false;
    bool isZeroBaseClipRange = false;
    bool isCenterPixelMapping = false;
};

void    Initialize( Api api, const InitParam& param );
void    Uninitialize();
void    Reset( const ResetParam& param );
bool    NeedRestoreResources();

void    Present(); // execute all submitted command-buffers & do flip/present

Api     HostApi();
bool    TextureFormatSupported( TextureFormat format );
const RenderDeviceCaps & DeviceCaps();

void    SuspendRendering();
void    ResumeRendering();

void    InvalidateCache();

    
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

HVertexBuffer   CreateVertexBuffer( const VertexBuffer::Descriptor& desc );
void            DeleteVertexBuffer(HVertexBuffer vb, bool forceImmediate = false);

void*           MapVertexBuffer( HVertexBuffer vb, uint32 offset, uint32 size );
void            UnmapVertexBuffer( HVertexBuffer vb );

void            UpdateVertexBuffer( HVertexBuffer vb, const void* data, uint32 offset, uint32 size );

bool            NeedRestoreVertexBuffer( HVertexBuffer vb );


////////////////////////////////////////////////////////////////////////////////
// index buffer

typedef ResourceHandle<RESOURCE_INDEX_BUFFER> HIndexBuffer;

HIndexBuffer    CreateIndexBuffer( const IndexBuffer::Descriptor& desc );
void            DeleteIndexBuffer(HIndexBuffer ib, bool forceImmediate = false);

void*           MapIndexBuffer( HIndexBuffer ib, uint32 offset, uint32 size );
void            UnmapIndexBuffer( HIndexBuffer ib );

void            UpdateIndexBuffer( HIndexBuffer ib, const void* data, uint32 offset, uint32 size );

bool            NeedRestoreIndexBuffer(HIndexBuffer vb);


////////////////////////////////////////////////////////////////////////////////
// query

typedef ResourceHandle<RESOURCE_QUERY_BUFFER> HQueryBuffer;

HQueryBuffer    CreateQueryBuffer( unsigned maxObjectCount );
void            ResetQueryBuffer( HQueryBuffer buf );
void            DeleteQueryBuffer(HQueryBuffer buf, bool forceImmediate = false);

bool            QueryIsReady( HQueryBuffer buf, uint32 objectIndex );
int             QueryValue( HQueryBuffer buf, uint32 objectIndex );


////////////////////////////////////////////////////////////////////////////////
// render-pipeline state & const-buffers

typedef ResourceHandle<RESOURCE_PIPELINE_STATE> HPipelineState;
typedef ResourceHandle<RESOURCE_CONST_BUFFER>   HConstBuffer;


HPipelineState  AcquireRenderPipelineState( const PipelineState::Descriptor& desc );
void            ReleaseRenderPipelineState(HPipelineState rps, bool forceImmediate = false);

HConstBuffer    CreateVertexConstBuffer( HPipelineState rps, uint32 bufIndex );
bool            CreateVertexConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf );

HConstBuffer    CreateFragmentConstBuffer( HPipelineState rps, uint32 bufIndex );
bool            CreateFragmentConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf );

bool            UpdateConstBuffer4fv( HConstBuffer constBuf, uint32 constIndex, const float* data, uint32 constCount );
bool            UpdateConstBuffer1fv( HConstBuffer constBuf, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount );
void            DeleteConstBuffer(HConstBuffer constBuf, bool forceImmediate = false);


////////////////////////////////////////////////////////////////////////////////
// texture-set

typedef ResourceHandle<RESOURCE_TEXTURE> HTexture;
typedef ResourceHandle<RESOURCE_TEXTURE_SET> HTextureSet;

HTexture        CreateTexture( const Texture::Descriptor& desc );
void            DeleteTexture(HTexture tex, bool forceImmediate = false);

void*           MapTexture( HTexture tex, uint32 level=0 );
void            UnmapTexture( HTexture tex );

void            UpdateTexture( HTexture tex, const void* data, uint32 level, TextureFace face=TEXTURE_FACE_NEGATIVE_X );

bool            NeedRestoreTexture( HTexture tex );


struct
TextureSetDescriptor
{
    uint32      fragmentTextureCount;
    HTexture    fragmentTexture[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32      vertexTextureCount;
    HTexture    vertexTexture[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
                
                TextureSetDescriptor()
                  : fragmentTextureCount(0),
                    vertexTextureCount(0)
                {}
};

HTextureSet     AcquireTextureSet( const TextureSetDescriptor& desc );
HTextureSet     CopyTextureSet( HTextureSet ts );
void            ReleaseTextureSet(HTextureSet ts, bool forceImmediate = false);
void            ReplaceTextureInAllTextureSets( HTexture oldHandle, HTexture newHandle );


////////////////////////////////////////////////////////////////////////////////
//  depthstencil-state

typedef ResourceHandle<RESOURCE_DEPTHSTENCIL_STATE> HDepthStencilState;

HDepthStencilState  AcquireDepthStencilState( const DepthStencilState::Descriptor& desc );
HDepthStencilState  CopyDepthStencilState( HDepthStencilState ds );
void                ReleaseDepthStencilState(HDepthStencilState ds, bool forceImmediate = false);


////////////////////////////////////////////////////////////////////////////////
//  sampler-state

typedef ResourceHandle<RESOURCE_SAMPLER_STATE> HSamplerState;

HSamplerState       AcquireSamplerState( const SamplerState::Descriptor& desc );
HSamplerState       CopySamplerState( HSamplerState ss );
void                ReleaseSamplerState(HSamplerState ss, bool forceImmediate = false);


////////////////////////////////////////////////////////////////////////////////
// sync-object

typedef ResourceHandle<RESOURCE_SYNC_OBJECT> HSyncObject;

HSyncObject         CreateSyncObject();
void                DeleteSyncObject( HSyncObject obj );
bool                SyncObjectSignaled( HSyncObject obj );


HSyncObject GetCurrentFrameSyncObject();

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
    enum { OPT_OVERRIDE_SCISSOR=1, OPT_WIREFRAME=2 };

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
    ScissorRect         scissorRect;
    uint32              vertexConstCount;
    HConstBuffer        vertexConst[MAX_CONST_BUFFER_COUNT];
    uint32              fragmentConstCount;
    HConstBuffer        fragmentConst[MAX_CONST_BUFFER_COUNT];
    HTextureSet         textureSet;
    PrimitiveType       primitiveType;
    uint32              primitiveCount;
    uint32              queryIndex;
    uint32              options;
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
                            queryIndex(InvalidIndex),
                            options(0),
                            debugMarker(nullptr)
                        {
                        }
};

void    BeginPacketList( HPacketList packetList );
void    AddPackets( HPacketList packetList, const Packet* packet, uint32 packetCount );
void    AddPacket( HPacketList packetList, const Packet& packet );
void    EndPacketList( HPacketList packetList, HSyncObject syncObject=HSyncObject(InvalidHandle) ); // 'packetList' handle invalid after this, no explicit "release" needed



} // namespace rhi
#endif // __RHI_PUBLIC_H__

