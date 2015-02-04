
    #include "../rhi_Manticore.h"
    #include "rhi_Pool.h"

namespace rhi
{

struct
TextureSet_t
{
    uint32  textureCount;
    Handle  texture[MAX_TEXTURE_SAMPLER_COUNT];
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

std::vector<TextureSetInfo> _TextureSetInfo;



struct
BatchDrawer_t
{
    Handle  cmdBuf;

    Handle  curPipelineState;
    Handle  curTextureSet;
};

typedef Pool<BatchDrawer_t,RESOURCE_BATCH_DRAWER> BatchDrawerPool;
RHI_IMPL_POOL(BatchDrawer_t,RESOURCE_BATCH_DRAWER);


//------------------------------------------------------------------------------

Handle
CreateVertexBuffer( uint32 size )
{
    return VertexBuffer::Create( size );
}


//------------------------------------------------------------------------------

void
DeleteVertexBuffer( Handle vb )
{
    VertexBuffer::Delete( vb );
}


//------------------------------------------------------------------------------

void*
MapVertexBuffer( Handle vb, uint32 offset, uint32 size )
{
    return VertexBuffer::Map( vb, offset, size );
}


//------------------------------------------------------------------------------

void
UnmapVertexBuffer( Handle vb )
{
    VertexBuffer::Unmap( vb );
}


//------------------------------------------------------------------------------

void
UpdateVertexBuffer( Handle vb, const void* data, uint32 offset, uint32 size )
{
    VertexBuffer::Update( vb, data, offset, size );
}


//------------------------------------------------------------------------------

Handle
CreateIndexBuffer( uint32 size )
{
    return IndexBuffer::Create( size );
}


//------------------------------------------------------------------------------

void
DeleteIndexBuffer( Handle vb )
{
    VertexBuffer::Delete( vb );
}


//------------------------------------------------------------------------------

void*
MapIndexBuffer( Handle ib, uint32 offset, uint32 size )
{
    return IndexBuffer::Map( ib, offset, size );
}


//------------------------------------------------------------------------------

void
UnmapIndexBuffer( Handle ib )
{
    IndexBuffer::Unmap( ib );
}


//------------------------------------------------------------------------------

void
UpdateIndexBuffer( Handle ib, const void* data, uint32 offset, uint32 size )
{
    IndexBuffer::Update( ib, data, offset, size );
}


//------------------------------------------------------------------------------
Handle 
AcquireRenderPipelineState( const PipelineState::Descriptor& desc )
{
    Handle  ps = InvalidHandle;

    if( ps == InvalidHandle )
    {
        ps = PipelineState::Create( desc );
    }
    
    return ps;
}


//------------------------------------------------------------------------------

void
ReleaseRenderPipelineState( Handle rps )
{
}


//------------------------------------------------------------------------------

bool
CreateVertexConstBuffers( Handle rps, uint32 maxCount, Handle* constBuf )
{
    bool    success = false;

    return success;
}


//------------------------------------------------------------------------------

bool
CreateFragmentConstBuffers( Handle rps, uint32 maxCount, Handle* constBuf )
{
    bool    success = false;

    return success;
}


//------------------------------------------------------------------------------

bool
UpdateConstBuffer( Handle constBuf, uint32 constIndex, const float* data, uint32 constCount )
{
    return ConstBuffer::SetConst( constBuf, constIndex, constCount, data );
}


//------------------------------------------------------------------------------

Handle
AcquireTextureSet( const TextureSetDescriptor& desc )
{
    Handle  handle = InvalidHandle;

    for( std::vector<TextureSetInfo>::const_iterator i=_TextureSetInfo.begin(),i_end=_TextureSetInfo.end(); i!=i_end; ++i )
    {
        if(     i->desc.count == desc.count 
            &&  memcmp( i->desc.texture, desc.texture, desc.count*sizeof(Handle) ) == 0
          )
        {
            TextureSet_t*   ts  = TextureSetPool::Get( i->handle );

            ++ts->refCount;
            
            handle = i->handle;
            break;
        }
    }
    
    if( handle == InvalidHandle )
    {
        handle = TextureSetPool::Alloc();

        TextureSet_t*   ts  = TextureSetPool::Get( handle );
        TextureSetInfo  info;

        ts->refCount     = 1;
        ts->textureCount = desc.count;
        memcpy( ts->texture, desc.texture, desc.count*sizeof(Handle) );
        
        info.desc   = desc;
        info.handle = handle;
        _TextureSetInfo.push_back( info );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
ReleaseTextureSet( Handle tsh )
{
    TextureSet_t*   ts  = TextureSetPool::Get( tsh );

    if( ts )
    {
        if( --ts->refCount == 0 )
        {
            TextureSetPool::Free( tsh );

            for( std::vector<TextureSetInfo>::const_iterator i=_TextureSetInfo.begin(),i_end=_TextureSetInfo.end(); i!=i_end; ++i )
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

Handle
AllocateRenderPass( const RenderPassConfig& passDesc, uint32 batchDrawerCount, Handle* batchDrawer )
{
    Handle       cb[8];
    DVASSERT(batchDrawerCount<countof(cb));
    Handle       pass = RenderPass::Allocate( passDesc, batchDrawerCount, cb );
    
    for( unsigned i=0; i!=batchDrawerCount; ++i )
    {
        Handle          bdh = BatchDrawerPool::Alloc();
        BatchDrawer_t*  bd  = BatchDrawerPool::Get( bdh );

        bd->cmdBuf = cb[i];


        batchDrawer[i] = bdh;
    }

    return pass;
}


//------------------------------------------------------------------------------

void
BeginRenderPass( Handle pass )
{
    RenderPass::Begin( pass );
}


//------------------------------------------------------------------------------

void
EndRenderPass( Handle pass )
{
    RenderPass::End( pass );
}


//------------------------------------------------------------------------------

void
BeginDraw( Handle batchDrawer )
{
    BatchDrawer_t*  bd  = BatchDrawerPool::Get( batchDrawer );

    bd->curPipelineState = InvalidHandle;
    bd->curTextureSet    = InvalidHandle;

    CommandBuffer::Begin( bd->cmdBuf );
}


//------------------------------------------------------------------------------

void
EndDraw( Handle batchDrawer )
{
    BatchDrawer_t*  bd  = BatchDrawerPool::Get( batchDrawer );

    CommandBuffer::End( bd->cmdBuf );
    BatchDrawerPool::Free( batchDrawer );
}


//------------------------------------------------------------------------------

void
DrawBatch( Handle batchDrawer, const BatchDescriptor& batchDesc )
{
    BatchDrawer_t*  bd      = BatchDrawerPool::Get( batchDrawer );
    Handle          cmdBuf  = bd->cmdBuf;

    if( batchDesc.renderPipelineState != bd->curPipelineState )
    {
        rhi::CommandBuffer::SetPipelineState( cmdBuf, batchDesc.renderPipelineState );
        bd->curPipelineState = batchDesc.renderPipelineState;
    }

    for( unsigned i=0; i!=batchDesc.vertexStreamCount; ++i )
    {
        rhi::CommandBuffer::SetVertexData( cmdBuf, batchDesc.vertexStream[i] );
    }
    
    if( batchDesc.indexBuffer != InvalidHandle )
    {
        rhi::CommandBuffer::SetIndices( cmdBuf, batchDesc.indexBuffer );
    }

    for( unsigned i=0; i!=batchDesc.vertexConstCount; ++i )
    {
        rhi::CommandBuffer::SetVertexConstBuffer( cmdBuf, i, batchDesc.vertexConst[i] );
    }

    for( unsigned i=0; i!=batchDesc.fragmentConstCount; ++i )
    {
        rhi::CommandBuffer::SetFragmentConstBuffer( cmdBuf, i, batchDesc.fragmentConst[i] );
    }

    if( batchDesc.textureSet != bd->curTextureSet )
    {
        TextureSet_t*   ts  = TextureSetPool::Get( batchDesc.textureSet );

        if( ts )
        {
            for( unsigned i=0; i!=ts->textureCount; ++i )
            {
                rhi::CommandBuffer::SetFragmentTexture( cmdBuf, i, ts->texture[i] );
            }
        }

        bd->curTextureSet = batchDesc.textureSet;
    }

    if( batchDesc.indexBuffer != InvalidHandle )
    {
        rhi::CommandBuffer::DrawIndexedPrimitive( cmdBuf, batchDesc.primitiveType, batchDesc.primitiveCount );
    }
    else
    {
        rhi::CommandBuffer::DrawPrimitive( cmdBuf, batchDesc.primitiveType, batchDesc.primitiveCount );
    }
}





} //namespace rhi
