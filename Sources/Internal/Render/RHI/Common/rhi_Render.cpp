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

    #include "../rhi_Public.h"
    #include "rhi_Private.h"
    #include "rhi_Pool.h"

    #include "Core/Core.h"    
    using DAVA::Logger;
    #include "Debug/Profiler.h"

namespace rhi
{
    
struct
TextureSet_t
{
    struct Desc {};

    uint32  fragmentTextureCount;
    Handle  fragmentTexture[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32  vertexTextureCount;
    Handle  vertexTexture[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    int     refCount;
};

typedef ResourcePool<TextureSet_t,RESOURCE_TEXTURE_SET,TextureSet_t::Desc,false> TextureSetPool;
RHI_IMPL_POOL(TextureSet_t,RESOURCE_TEXTURE_SET,TextureSet_t::Desc,false);

struct
TextureSetInfo
{
    TextureSetDescriptor    desc;
    Handle                  handle;
};


struct
DepthStencilState_t
{
    DepthStencilState::Descriptor   desc;
    Handle                          state;
    int                             refCount;
};

struct
SamplerState_t
{
    SamplerState::Descriptor    desc;
    Handle                      state;
    int                         refCount;
};


static std::vector<TextureSetInfo>      _TextureSetInfo;
static std::vector<DepthStencilState_t> _DepthStencilStateInfo;
static std::vector<SamplerState_t>      _SamplerStateInfo;

struct ScheduledDeleteResource 
{
    Handle handle;    
    ResourceType resourceType;
};

const static uint32 frameSyncObjectsCount = 16;
static uint32 currFrameSyncId = 0;
static std::array<HSyncObject, frameSyncObjectsCount> frameSyncObjects;
static std::array<std::vector<ScheduledDeleteResource>, frameSyncObjectsCount> scheduledDeleteResources;
DAVA::Mutex sheduledDeleteMutex;

inline void AddSheduletDeleteResource(Handle handle, ResourceType resourceType)
{
    sheduledDeleteMutex.Lock();
    scheduledDeleteResources[currFrameSyncId].push_back({ handle, resourceType });
    sheduledDeleteMutex.Unlock();
}

struct
PacketList_t
{
    struct Desc {};

    Handle      cmdBuf;
    Handle      queryBuffer;
    Viewport    viewport;

    Handle      curPipelineState;
    uint32      curVertexLayout;
    Handle      curTextureSet;
    Handle      curSamplerState;
    Handle      curDepthStencilState;
    CullMode    curCullMode;

    Handle      defDepthStencilState;
    Handle      defSamplerState;
    ScissorRect defScissorRect;

    Handle      curVertexStream[MAX_VERTEX_STREAM_COUNT];

    uint32      setDefaultViewport:1;
    uint32      restoreDefScissorRect:1;
    uint32      invertCulling:1;

    // debug
    uint32      batchIndex;
};

typedef ResourcePool<PacketList_t,RESOURCE_PACKET_LIST,PacketList_t::Desc,false>    PacketListPool;
RHI_IMPL_POOL(PacketList_t,RESOURCE_PACKET_LIST,PacketList_t::Desc,false);


//------------------------------------------------------------------------------

HVertexBuffer
CreateVertexBuffer( const VertexBuffer::Descriptor& desc )
{
    return HVertexBuffer(VertexBuffer::Create( desc ));
}


//------------------------------------------------------------------------------

void
DeleteVertexBuffer(HVertexBuffer vb, bool forceImmediate)
{           
    if (forceImmediate)
        VertexBuffer::Delete(vb);
    else
        AddSheduletDeleteResource( vb, RESOURCE_VERTEX_BUFFER );
}


//------------------------------------------------------------------------------

void*
MapVertexBuffer( HVertexBuffer vb, uint32 offset, uint32 size )
{
    return VertexBuffer::Map( vb, offset, size );
}


//------------------------------------------------------------------------------

void
UnmapVertexBuffer( HVertexBuffer vb )
{
    VertexBuffer::Unmap( vb );
}


//------------------------------------------------------------------------------

void
UpdateVertexBuffer( HVertexBuffer vb, const void* data, uint32 offset, uint32 size )
{
    VertexBuffer::Update( vb, data, offset, size );
}


//------------------------------------------------------------------------------

bool
NeedRestoreVertexBuffer( HVertexBuffer vb )
{
    return VertexBuffer::NeedRestore( vb );
}


//------------------------------------------------------------------------------

HIndexBuffer
CreateIndexBuffer( const IndexBuffer::Descriptor& desc )
{
    return HIndexBuffer(IndexBuffer::Create( desc ));
}


//------------------------------------------------------------------------------

void
DeleteIndexBuffer(HIndexBuffer ib, bool forceImmediate)
{
    if (forceImmediate)
        IndexBuffer::Delete(ib);
    else
        AddSheduletDeleteResource( ib, RESOURCE_INDEX_BUFFER );
}


//------------------------------------------------------------------------------

void*
MapIndexBuffer( HIndexBuffer ib, uint32 offset, uint32 size )
{
    return IndexBuffer::Map( ib, offset, size );
}


//------------------------------------------------------------------------------

void
UnmapIndexBuffer( HIndexBuffer ib )
{
    IndexBuffer::Unmap( ib );
}


//------------------------------------------------------------------------------

void
UpdateIndexBuffer( HIndexBuffer ib, const void* data, uint32 offset, uint32 size )
{
    IndexBuffer::Update( ib, data, offset, size );
}


//------------------------------------------------------------------------------

bool
NeedRestoreIndexBuffer( HIndexBuffer ib )
{
    return IndexBuffer::NeedRestore( ib );
}


//------------------------------------------------------------------------------

HQueryBuffer
CreateQueryBuffer( uint32 maxObjectCount )
{
    return HQueryBuffer(QueryBuffer::Create( maxObjectCount ));
}


//------------------------------------------------------------------------------

void
ResetQueryBuffer( HQueryBuffer buf )
{
    QueryBuffer::Reset( buf );
}


//------------------------------------------------------------------------------

void
DeleteQueryBuffer(HQueryBuffer buf, bool forceImmediate)
{    
    if (forceImmediate)
        QueryBuffer::Delete(buf);
    else
        AddSheduletDeleteResource( buf, RESOURCE_QUERY_BUFFER );
}


//------------------------------------------------------------------------------

bool
QueryIsReady( HQueryBuffer buf, uint32 objectIndex )
{
    return QueryBuffer::IsReady( buf, objectIndex );
}


//------------------------------------------------------------------------------

int
QueryValue( HQueryBuffer buf, uint32 objectIndex )
{
    return QueryBuffer::Value( buf, objectIndex );
}


//------------------------------------------------------------------------------

HPipelineState 
AcquireRenderPipelineState( const PipelineState::Descriptor& desc )
{
    HPipelineState  ps;

    if( !ps.IsValid() )
    {
        ps = HPipelineState(PipelineState::Create( desc ));
    }
    
    return ps;
}


//------------------------------------------------------------------------------

void
ReleaseRenderPipelineState(HPipelineState rps, bool forceImmediate)
{
//    PipelineState::Delete( rps );
}


//------------------------------------------------------------------------------

HConstBuffer
CreateVertexConstBuffer( HPipelineState rps, uint32 bufIndex )
{
    return HConstBuffer(PipelineState::CreateVertexConstBuffer( rps, bufIndex ));
}


//------------------------------------------------------------------------------

bool
CreateVertexConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf )
{
    bool    success = false;

    for( unsigned i=0; i!=maxCount; ++i )
        constBuf[i] = HConstBuffer(PipelineState::CreateVertexConstBuffer( rps, i ));

    return success;
}


//------------------------------------------------------------------------------

HConstBuffer
CreateFragmentConstBuffer( HPipelineState rps, uint32 bufIndex )
{
    return HConstBuffer(PipelineState::CreateFragmentConstBuffer( rps, bufIndex ));
}


//------------------------------------------------------------------------------

bool
CreateFragmentConstBuffers( HPipelineState rps, uint32 maxCount, HConstBuffer* constBuf )
{
    bool    success = false;

    for( unsigned i=0; i!=maxCount; ++i )
        constBuf[i] = HConstBuffer(PipelineState::CreateFragmentConstBuffer( rps, i ));

    return success;
}


//------------------------------------------------------------------------------

bool
UpdateConstBuffer4fv( HConstBuffer constBuf, uint32 constIndex, const float* data, uint32 constCount )
{
    return ConstBuffer::SetConst( constBuf, constIndex, constCount, data );
}


//------------------------------------------------------------------------------

bool
UpdateConstBuffer1fv( HConstBuffer constBuf, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount )
{
    return ConstBuffer::SetConst( constBuf, constIndex, constSubIndex, data, dataCount );
}


//------------------------------------------------------------------------------

void
DeleteConstBuffer(HConstBuffer constBuf, bool forceImmediate)
{
    if (forceImmediate)
        ConstBuffer::Delete(constBuf);
    else
        AddSheduletDeleteResource( constBuf, RESOURCE_CONST_BUFFER );
}


//------------------------------------------------------------------------------

HTexture
CreateTexture( const Texture::Descriptor& desc )
{
    return HTexture(Texture::Create( desc ));
}


//------------------------------------------------------------------------------

void
DeleteTexture(HTexture tex, bool forceImmediate)
{
    if (forceImmediate)
        Texture::Delete(tex);
    else
        AddSheduletDeleteResource( tex, RESOURCE_TEXTURE );
}


//------------------------------------------------------------------------------

void*
MapTexture( HTexture tex, uint32 level )
{
    return Texture::Map( tex, level );
}


//------------------------------------------------------------------------------

void
UnmapTexture( HTexture tex )
{
    Texture::Unmap( tex );
}


//------------------------------------------------------------------------------

void
UpdateTexture( HTexture tex, const void* data, uint32 level, TextureFace face )
{
    Texture::Update( tex, data, level, face );
}


//------------------------------------------------------------------------------

bool
NeedRestoreTexture( HTexture tex )
{
    return Texture::NeedRestore( tex );
}


//------------------------------------------------------------------------------

HTextureSet
AcquireTextureSet( const TextureSetDescriptor& desc )
{
    HTextureSet handle;

    for( std::vector<TextureSetInfo>::const_iterator i=_TextureSetInfo.begin(),i_end=_TextureSetInfo.end(); i!=i_end; ++i )
    {
        if(     i->desc.fragmentTextureCount == desc.fragmentTextureCount 
            &&  i->desc.vertexTextureCount == desc.vertexTextureCount
            &&  memcmp( i->desc.fragmentTexture, desc.fragmentTexture, desc.fragmentTextureCount*sizeof(Handle) ) == 0
            &&  memcmp( i->desc.vertexTexture, desc.vertexTexture, desc.vertexTextureCount*sizeof(Handle) ) == 0
          )
        {
            TextureSet_t*   ts  = TextureSetPool::Get( i->handle );

            ++ts->refCount;
            
            handle = HTextureSet(i->handle);
            break;
        }
    }
    
    if( !handle.IsValid() )
    {
        handle = HTextureSet(TextureSetPool::Alloc());

        TextureSet_t*   ts  = TextureSetPool::Get( handle );
        TextureSetInfo  info;

        ts->refCount             = 1;
        ts->fragmentTextureCount = desc.fragmentTextureCount;
        ts->vertexTextureCount   = desc.vertexTextureCount;
        memcpy( ts->fragmentTexture, desc.fragmentTexture, desc.fragmentTextureCount*sizeof(Handle) );
        memcpy( ts->vertexTexture, desc.vertexTexture, desc.vertexTextureCount*sizeof(Handle) );
        
        info.desc   = desc;
        info.handle = handle;
        _TextureSetInfo.push_back( info );
    }

    return handle;
}


//------------------------------------------------------------------------------

HTextureSet
CopyTextureSet( HTextureSet tsh )
{
    HTextureSet     handle;
    TextureSet_t*   ts     = TextureSetPool::Get( tsh );

    if( ts )
    {
        ++ts->refCount;
        handle = tsh;
    }

    return handle;
}


//------------------------------------------------------------------------------

void
ReleaseTextureSet(HTextureSet tsh, bool forceImmediate)
{
    TextureSet_t*   ts  = TextureSetPool::Get( tsh );

    if( ts )
    {
        if( --ts->refCount == 0 )
        {
            if (forceImmediate)
                TextureSetPool::Free(tsh);
            else
                AddSheduletDeleteResource( tsh, RESOURCE_TEXTURE_SET );

            for( std::vector<TextureSetInfo>::iterator i=_TextureSetInfo.begin(),i_end=_TextureSetInfo.end(); i!=i_end; ++i )
            {
                if( i->handle == tsh )
                {
                    _TextureSetInfo.erase( i );
                    break;
                }
            }
        }
    }
}


//------------------------------------------------------------------------------

void
ReplaceTextureInAllTextureSets( HTexture oldHandle, HTexture newHandle )
{
	for( std::vector<TextureSetInfo>::iterator s=_TextureSetInfo.begin(), s_end=_TextureSetInfo.end(); s!=s_end; ++s )
	{
        // update texture-set itself

        TextureSet_t*   ts  = TextureSetPool::Get( s->handle );

        if( ts )
        {
            for( unsigned i=0; i!=ts->fragmentTextureCount; ++i )
            {
                if( ts->fragmentTexture[i] == oldHandle )
                    ts->fragmentTexture[i] = newHandle;
            }
            for( unsigned i=0; i!=ts->vertexTextureCount; ++i )
            {
                if( ts->vertexTexture[i] == oldHandle )
                    ts->vertexTexture[i] = newHandle;
            }
        }                                                

        
        // update desc as well

		for( uint32 t=0; t!=s->desc.fragmentTextureCount; ++t )
        {
            if( s->desc.fragmentTexture[t] == oldHandle )
                s->desc.fragmentTexture[t] = newHandle;
        }
		for( uint32 t=0; t!=s->desc.vertexTextureCount; ++t )
        {
            if( s->desc.vertexTexture[t] == oldHandle )
                s->desc.vertexTexture[t] = newHandle;
        }
	}
}


//------------------------------------------------------------------------------

HDepthStencilState
AcquireDepthStencilState( const DepthStencilState::Descriptor& desc )
{
    Handle  ds = InvalidHandle;

    for( std::vector<DepthStencilState_t>::iterator i=_DepthStencilStateInfo.begin(),i_end=_DepthStencilStateInfo.end(); i!=i_end; ++i )
    {
        if( memcmp( &(i->desc), &desc, sizeof(DepthStencilState::Descriptor) ) == 0 )
        {
            ds = i->state;
            ++i->refCount;
            break;
        }
    }
    
    if( ds == InvalidHandle )
    {
        DepthStencilState_t info;
        
        info.desc       = desc;
        info.state      = DepthStencilState::Create( desc );
        info.refCount   = 1;

        _DepthStencilStateInfo.push_back( info );
        ds = info.state;
    }

    return HDepthStencilState(ds);
}


//------------------------------------------------------------------------------

HDepthStencilState
CopyDepthStencilState( HDepthStencilState ds )
{
    HDepthStencilState  handle;

    for( std::vector<DepthStencilState_t>::iterator i=_DepthStencilStateInfo.begin(),i_end=_DepthStencilStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ds )
        {
            ++i->refCount;
            handle = ds;
            break;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void
ReleaseDepthStencilState(HDepthStencilState ds, bool forceImmediate)
{
    for( std::vector<DepthStencilState_t>::iterator i=_DepthStencilStateInfo.begin(),i_end=_DepthStencilStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ds )
        {
            if( --i->refCount == 0 )
            {
                if (forceImmediate)
                    DepthStencilState::Delete(i->state);
                else
                    AddSheduletDeleteResource( i->state, RESOURCE_DEPTHSTENCIL_STATE );
                _DepthStencilStateInfo.erase( i );
            }

            break;
        }
    }
}


//------------------------------------------------------------------------------

HSamplerState
AcquireSamplerState( const SamplerState::Descriptor& desc )
{
    Handle  ss = InvalidHandle;

    for( std::vector<SamplerState_t>::iterator i=_SamplerStateInfo.begin(),i_end=_SamplerStateInfo.end(); i!=i_end; ++i )
    {
        if( memcmp( &(i->desc), &desc, sizeof(SamplerState::Descriptor) ) == 0 )
        {
            ss = i->state;
            ++i->refCount;
            break;
        }
    }
    
    if( ss == InvalidHandle )
    {
        SamplerState_t info;
        
        info.desc       = desc;
        info.state      = SamplerState::Create( desc );
        info.refCount   = 1;

        _SamplerStateInfo.push_back( info );
        ss = info.state;
    }

    return HSamplerState(ss);
}



//------------------------------------------------------------------------------

HSamplerState
CopySamplerState( HSamplerState ss )
{
    Handle  handle = InvalidHandle;

    for( std::vector<SamplerState_t>::iterator i=_SamplerStateInfo.begin(),i_end=_SamplerStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ss )
        {
            ++i->refCount;
            handle = i->state;
            break;
        }
    }

    return HSamplerState(handle);
}


//------------------------------------------------------------------------------

void
ReleaseSamplerState(HSamplerState ss, bool forceImmediate)
{
    for( std::vector<SamplerState_t>::iterator i=_SamplerStateInfo.begin(),i_end=_SamplerStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ss )
        {
            if( --i->refCount == 0 )
            {
                if (forceImmediate)
                    SamplerState::Delete(i->state);
                else                
                    AddSheduletDeleteResource( i->state, RESOURCE_SAMPLER_STATE );
                _SamplerStateInfo.erase( i );
            }

            break;
        }
    }
}


//------------------------------------------------------------------------------

HSyncObject
CreateSyncObject()
{
    return HSyncObject(SyncObject::Create());
}


//------------------------------------------------------------------------------

void
DeleteSyncObject( HSyncObject obj )
{
    SyncObject::Delete( obj );
}


//------------------------------------------------------------------------------

bool
SyncObjectSignaled( HSyncObject obj )
{
    return SyncObject::IsSygnaled( obj );
}


//------------------------------------------------------------------------------

HRenderPass
AllocateRenderPass( const RenderPassConfig& passDesc, uint32 packetListCount, HPacketList* packetList )
{
    Handle       cb[8];
    DVASSERT(packetListCount<countof(cb));

    Handle       pass = RenderPass::Allocate( passDesc, packetListCount, cb );
    
    for( unsigned i=0; i!=packetListCount; ++i )
    {
        Handle          plh = PacketListPool::Alloc();
        PacketList_t*   pl  = PacketListPool::Get( plh );

        pl->cmdBuf              = cb[i];
        pl->queryBuffer         = passDesc.queryBuffer;
        pl->setDefaultViewport  = i == 0;
        pl->viewport            = passDesc.viewport;
        pl->invertCulling       = passDesc.invertCulling;


        packetList[i] = HPacketList(plh);
    }

    return HRenderPass(pass);
}


//------------------------------------------------------------------------------

void
BeginRenderPass( HRenderPass pass )
{
    RenderPass::Begin( pass );
}


//------------------------------------------------------------------------------

void
EndRenderPass( HRenderPass pass )
{
    RenderPass::End( pass );
}


//------------------------------------------------------------------------------

void
BeginPacketList( HPacketList packetList )
{
    PacketList_t*   pl     = PacketListPool::Get( packetList );
    static Handle   def_ds = rhi::InvalidHandle;
    static Handle   def_ss = rhi::InvalidHandle;

    if( def_ds == rhi::InvalidHandle )
    {
        rhi::DepthStencilState::Descriptor  desc;
        
        def_ds = rhi::DepthStencilState::Create( desc );
    }
    
    if( def_ss == rhi::InvalidHandle )
    {
        rhi::SamplerState::Descriptor   desc;

        desc.fragmentSamplerCount = rhi::MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT;
        desc.vertexSamplerCount   = rhi::MAX_VERTEX_TEXTURE_SAMPLER_COUNT;
        def_ss                    = rhi::SamplerState::Create( desc );
    }
    

    pl->curPipelineState      = InvalidHandle;
    pl->curVertexLayout       = rhi::VertexLayout::InvalidUID;
    pl->curTextureSet         = InvalidHandle;
    pl->defDepthStencilState  = def_ds;
    pl->defSamplerState       = def_ss;

    CommandBuffer::Begin( pl->cmdBuf );
    
    if( pl->setDefaultViewport )
        CommandBuffer::SetViewport( pl->cmdBuf, pl->viewport );
    
    CommandBuffer::SetScissorRect( pl->cmdBuf, ScissorRect() ); // ensure default scissor-rect is used

    CommandBuffer::SetDepthStencilState( pl->cmdBuf, pl->defDepthStencilState );
    pl->curDepthStencilState = pl->defDepthStencilState;
    
    CommandBuffer::SetSamplerState( pl->cmdBuf, def_ss );
    pl->curSamplerState = pl->defSamplerState;

    CommandBuffer::SetCullMode( pl->cmdBuf, CULL_NONE );
    pl->curCullMode = CULL_NONE;

    for( unsigned i=0; i!=countof(pl->curVertexStream); ++i )
        pl->curVertexStream[i] = InvalidHandle;
    

    CommandBuffer::SetCullMode( pl->cmdBuf, CULL_NONE );
    
    if( pl->queryBuffer != rhi::InvalidHandle )
        CommandBuffer::SetQueryBuffer( pl->cmdBuf, pl->queryBuffer );

    pl->restoreDefScissorRect = false;

    pl->batchIndex = 0;
}


//------------------------------------------------------------------------------

void
EndPacketList( HPacketList packetList, HSyncObject syncObject )
{
    PacketList_t*   pl  = PacketListPool::Get( packetList );

    CommandBuffer::End( pl->cmdBuf, syncObject );
    PacketListPool::Free( packetList );
}


//------------------------------------------------------------------------------

void
AddPackets( HPacketList packetList, const Packet* packet, uint32 packetCount )
{
SCOPED_NAMED_TIMING("rhi.AddPackets");
    PacketList_t*   pl      = PacketListPool::Get( packetList );
    Handle          cmdBuf  = pl->cmdBuf;
    
    for( const Packet* p=packet,*p_end=packet+packetCount; p!=p_end; ++p )
    {
        Handle          dsState = (p->depthStencilState != rhi::InvalidHandle)
                                  ? p->depthStencilState
                                  : pl->defDepthStencilState;
        Handle          sState  = (p->samplerState != rhi::InvalidHandle)
                                  ? p->samplerState
                                  : pl->defSamplerState;

        if( p->debugMarker )
        {
            rhi::CommandBuffer::SetMarker( cmdBuf, p->debugMarker );
        }
        
        if(     p->renderPipelineState != pl->curPipelineState 
            ||  p->vertexLayoutUID != pl->curVertexLayout
          )
        {
            rhi::CommandBuffer::SetPipelineState( cmdBuf, p->renderPipelineState, p->vertexLayoutUID );
            pl->curPipelineState = p->renderPipelineState;
            pl->curVertexLayout  = p->vertexLayoutUID;
        }

        if( dsState != pl->curDepthStencilState )
        {
            rhi::CommandBuffer::SetDepthStencilState( cmdBuf, dsState );
            pl->curDepthStencilState = p->depthStencilState;
        }
        if( sState != pl->curSamplerState )
        {
            rhi::CommandBuffer::SetSamplerState( cmdBuf, sState );
            pl->curSamplerState = p->samplerState;
        }
        if( p->cullMode !=  pl->curCullMode )
        {
            CullMode    mode = p->cullMode;

            if( pl->invertCulling )
            {
                switch( mode )
                {
                    case CULL_CW  : mode = CULL_CCW; break;
                    case CULL_CCW : mode = CULL_CW; break;
                    default: break;
                }
            }

            rhi::CommandBuffer::SetCullMode( cmdBuf, mode );
            pl->curCullMode = p->cullMode;
        }

        for( unsigned i=0; i!=p->vertexStreamCount; ++i )
        {
//-            if( p->vertexStream[i] != pl->curVertexStream[i] )
            {
                rhi::CommandBuffer::SetVertexData( cmdBuf, p->vertexStream[i] );
                pl->curVertexStream[i] = p->vertexStream[i];
            }
        }
    
        if( p->indexBuffer != InvalidHandle )
        {
            rhi::CommandBuffer::SetIndices( cmdBuf, p->indexBuffer );
        }

        for( unsigned i=0; i!=p->vertexConstCount; ++i )
        {
            rhi::CommandBuffer::SetVertexConstBuffer( cmdBuf, i, p->vertexConst[i] );
        }

        for( unsigned i=0; i!=p->fragmentConstCount; ++i )
        {
            rhi::CommandBuffer::SetFragmentConstBuffer( cmdBuf, i, p->fragmentConst[i] );
        }

        if( p->textureSet != pl->curTextureSet )
        {
            TextureSet_t*   ts  = TextureSetPool::Get( p->textureSet );

            if( ts )
            {
                for( unsigned i=0; i!=ts->fragmentTextureCount; ++i )
                {
                    rhi::CommandBuffer::SetFragmentTexture( cmdBuf, i, ts->fragmentTexture[i] );
                }
                for( unsigned i=0; i!=ts->vertexTextureCount; ++i )
                {
                    rhi::CommandBuffer::SetVertexTexture( cmdBuf, i, ts->vertexTexture[i] );
                }
            }                                                

            pl->curTextureSet = p->textureSet;
        }

        if( p->options & Packet::OPT_OVERRIDE_SCISSOR )
        {
            DVASSERT(p->scissorRect.x>=0);
            DVASSERT(p->scissorRect.y>=0);
            DVASSERT(p->scissorRect.width>=0);
            DVASSERT(p->scissorRect.height>=0);

            rhi::CommandBuffer::SetScissorRect( cmdBuf, p->scissorRect );
            pl->restoreDefScissorRect = true;
        }
        else
        {
            if( pl->restoreDefScissorRect )
            {
                rhi::CommandBuffer::SetScissorRect( cmdBuf, pl->defScissorRect );
                pl->restoreDefScissorRect = false;
            }
        }

        
//        if( p->queryIndex != InvalidIndex )
        {
            rhi::CommandBuffer::SetQueryIndex( cmdBuf, p->queryIndex );
        }        

        if( p->indexBuffer != InvalidHandle )
        {
            DVASSERT(p->vertexCount); // vertexCount MUST BE SPECIFIED
            rhi::CommandBuffer::DrawIndexedPrimitive( cmdBuf, p->primitiveType, p->primitiveCount, p->vertexCount, p->baseVertex, p->startIndex );
        }
        else
        {
            rhi::CommandBuffer::DrawPrimitive( cmdBuf, p->primitiveType, p->primitiveCount );
        }

        ++pl->batchIndex;
    }
}



//------------------------------------------------------------------------------

void
AddPacket( HPacketList packetList, const Packet& packet )
{
    AddPackets( packetList, &packet, 1 );
}


/**/

void ProcessScheduledDelete()
{        
    for (int i = 0; i < frameSyncObjectsCount; i++)
    {
        if (frameSyncObjects[i].IsValid() && SyncObjectSignaled(frameSyncObjects[i]))
        {
            for (std::vector<ScheduledDeleteResource>::iterator it = scheduledDeleteResources[i].begin(), e = scheduledDeleteResources[i].end(); it != e; ++it)            
            {
                ScheduledDeleteResource& res = *it;
                switch (res.resourceType)
                {
                case RESOURCE_VERTEX_BUFFER: //+
                    VertexBuffer::Delete(res.handle);                    
                    break;
                case RESOURCE_INDEX_BUFFER: //+
                    IndexBuffer::Delete(res.handle);
                    break;
                case RESOURCE_TEXTURE: //+
                    Texture::Delete(res.handle);
                    break;
                case RESOURCE_QUERY_BUFFER: //+
                    QueryBuffer::Delete(res.handle);
                    break;
                case RESOURCE_CONST_BUFFER: //+
                    ConstBuffer::Delete(res.handle);
                    break;
                case RESOURCE_PIPELINE_STATE: //-
                    PipelineState::Delete(res.handle);
                    break;
                case RESOURCE_DEPTHSTENCIL_STATE: //+
                    DepthStencilState::Delete(res.handle);
                    break;
                case RESOURCE_SAMPLER_STATE: //+
                    SamplerState::Delete(res.handle);
                    break;
                case RESOURCE_TEXTURE_SET:
                    TextureSetPool::Free(res.handle);                    
                    break;                        
                default:
                    DVASSERT_MSG(false, "Not supported resource scheduled for deletion");
                }
            }
            scheduledDeleteResources[i].clear();
        }
    }
}

void Present()
{        
    sheduledDeleteMutex.Lock();
    if (scheduledDeleteResources[currFrameSyncId].size()&&!frameSyncObjects[currFrameSyncId].IsValid())
        frameSyncObjects[currFrameSyncId] = CreateSyncObject();

    PresentImpl(frameSyncObjects[currFrameSyncId]);                

    currFrameSyncId = (currFrameSyncId + 1) % frameSyncObjectsCount;
    DVASSERT(scheduledDeleteResources[currFrameSyncId].empty()); //we are not going to mix new resources for deletion with existing once still waiting    
    if (frameSyncObjects[currFrameSyncId].IsValid())
    {
        DeleteSyncObject(frameSyncObjects[currFrameSyncId]);
        frameSyncObjects[currFrameSyncId] = HSyncObject();
    }

    ProcessScheduledDelete();
    sheduledDeleteMutex.Unlock();
}

HSyncObject GetCurrentFrameSyncObject()
{
    if (!frameSyncObjects[currFrameSyncId].IsValid())
        frameSyncObjects[currFrameSyncId] = CreateSyncObject();

    return frameSyncObjects[currFrameSyncId];
}



} //namespace rhi
