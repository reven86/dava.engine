
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
    uint32  fragmentTextureCount;
    Handle  fragmentTexture[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32  vertexTextureCount;
    Handle  vertexTexture[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    int     refCount;
};

typedef Pool<TextureSet_t,RESOURCE_TEXTURE_SET> TextureSetPool;
RHI_IMPL_POOL(TextureSet_t,RESOURCE_TEXTURE_SET);

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



struct
PacketList_t
{
    Handle      cmdBuf;

    Handle      curPipelineState;
    uint32      curVertexLayout;
    Handle      curTextureSet;
    Handle      curSamplerState;
    Handle      curDepthStencilState;
    CullMode    curCullMode;

    Handle      defDepthStencilState;
    Handle      defSamplerState;

    Handle      curVertexStream[MAX_VERTEX_STREAM_COUNT];

    // debug
    uint32  batchIndex;
};

typedef Pool<PacketList_t,RESOURCE_PACKET_LIST>     PacketListPool;
RHI_IMPL_POOL(PacketList_t,RESOURCE_PACKET_LIST);


//------------------------------------------------------------------------------

HVertexBuffer
CreateVertexBuffer( uint32 size )
{
    return HVertexBuffer(VertexBuffer::Create( size ));
}


//------------------------------------------------------------------------------

void
DeleteVertexBuffer( HVertexBuffer vb )
{
    VertexBuffer::Delete( vb );
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

HIndexBuffer
CreateIndexBuffer( uint32 size )
{
    return HIndexBuffer(IndexBuffer::Create( size ));
}


//------------------------------------------------------------------------------

void
DeleteIndexBuffer( HIndexBuffer ib )
{
    IndexBuffer::Delete( ib );
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
ReleaseRenderPipelineState( HPipelineState rps )
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
DeleteConstBuffer( HConstBuffer constBuf )
{
    ConstBuffer::Delete( constBuf );
}


//------------------------------------------------------------------------------

HTexture
CreateTexture( const Texture::Descriptor& desc )
{
    return HTexture(Texture::Create( desc ));
}


//------------------------------------------------------------------------------

void
DeleteTexture( HTexture tex )
{
    Texture::Delete( tex );
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
ReleaseTextureSet( HTextureSet tsh )
{
    TextureSet_t*   ts  = TextureSetPool::Get( tsh );

    if( ts )
    {
        if( --ts->refCount == 0 )
        {
            TextureSetPool::Free( tsh );

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
ReleaseDepthStencilState( HDepthStencilState ds )
{
    for( std::vector<DepthStencilState_t>::iterator i=_DepthStencilStateInfo.begin(),i_end=_DepthStencilStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ds )
        {
            if( --i->refCount == 0 )
            {
                DepthStencilState::Delete( i->state );
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
ReleaseSamplerState( HSamplerState ss )
{
    for( std::vector<SamplerState_t>::iterator i=_SamplerStateInfo.begin(),i_end=_SamplerStateInfo.end(); i!=i_end; ++i )
    {
        if( i->state == ss )
        {
            if( --i->refCount == 0 )
            {
                SamplerState::Delete( i->state );
                _SamplerStateInfo.erase( i );
            }

            break;
        }
    }
}


//------------------------------------------------------------------------------

HRenderPass
AllocateRenderPass( const RenderPassConfig& passDesc, uint32 packetListCount, HPacketList* packetList )
{
    Handle       cb[8];
    DVASSERT(packetListCount<countof(cb));
    DVASSERT(passDesc.viewport[2] > 0 && passDesc.viewport[3] > 0);

    Handle       pass = RenderPass::Allocate( passDesc, packetListCount, cb );
    
    for( unsigned i=0; i!=packetListCount; ++i )
    {
        Handle          plh = PacketListPool::Alloc();
        PacketList_t*   pl  = PacketListPool::Get( plh );

        pl->cmdBuf = cb[i];


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
        def_ss                    = rhi::SamplerState::Create( desc );
    }
    

    pl->curPipelineState      = InvalidHandle;
    pl->curVertexLayout       = rhi::VertexLayout::InvalidUID;
    pl->curTextureSet         = InvalidHandle;
    pl->defDepthStencilState  = def_ds;
    pl->defSamplerState       = def_ss;

    CommandBuffer::Begin( pl->cmdBuf );

    CommandBuffer::SetDepthStencilState( pl->cmdBuf, pl->defDepthStencilState );
    pl->curDepthStencilState = pl->defDepthStencilState;
    
    CommandBuffer::SetSamplerState( pl->cmdBuf, def_ss );
    pl->curSamplerState = pl->defSamplerState;

    CommandBuffer::SetCullMode( pl->cmdBuf, CULL_NONE );
    pl->curCullMode = CULL_NONE;

    for( unsigned i=0; i!=countof(pl->curVertexStream); ++i )
        pl->curVertexStream[i] = InvalidHandle;


    pl->batchIndex = 0;
}


//------------------------------------------------------------------------------

void
EndPacketList( HPacketList packetList )
{
    PacketList_t*   pl  = PacketListPool::Get( packetList );

    CommandBuffer::End( pl->cmdBuf );
    PacketListPool::Free( packetList );
}


//------------------------------------------------------------------------------

void
AddPackets( HPacketList packetList, const Packet* packet, uint32 packetCount )
{
    PacketList_t*   pl      = PacketListPool::Get( packetList );
    Handle          cmdBuf  = pl->cmdBuf;
    
    for( const Packet* p=packet,*p_end=packet+packetCount; p!=p_end; ++p )
    {
        SCOPED_NAMED_TIMING("rhi.DrawBatch");
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
            rhi::CommandBuffer::SetCullMode( cmdBuf, p->cullMode );
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





} //namespace rhi
