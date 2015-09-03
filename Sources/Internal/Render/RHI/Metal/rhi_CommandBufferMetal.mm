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

    #include "../rhi_Type.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Core/Core.h"
    #include "Debug/Profiler.h"

    #include "_metal.h"


namespace rhi
{

struct
RenderPassMetal_t
{
    MTLRenderPassDescriptor*            desc;
    id<MTLCommandBuffer>                buf;
    id<MTLParallelRenderCommandEncoder> encoder;
    std::vector<Handle>                 cmdBuf;
    int                                 priority;
};


struct
CommandBufferMetal_t
{
    id<MTLRenderCommandEncoder> encoder;
    id<MTLCommandBuffer>        buf;

    id<MTLTexture>              rt;
    Handle                      cur_ib;
};


struct
SyncObjectMetal_t 
{
    uint32  is_signaled:1;
};

typedef ResourcePool<CommandBufferMetal_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false>  CommandBufferPool;
typedef ResourcePool<RenderPassMetal_t,RESOURCE_RENDER_PASS,RenderPassConfig,false>                 RenderPassPool;
typedef ResourcePool<SyncObjectMetal_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false>           SyncObjectPool;

RHI_IMPL_POOL(CommandBufferMetal_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false);
RHI_IMPL_POOL(RenderPassMetal_t,RESOURCE_RENDER_PASS,RenderPassConfig,false);
RHI_IMPL_POOL(SyncObjectMetal_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false);

static id<CAMetalDrawable>      _CurDrawable = nil;    
static std::vector<Handle>      _CmdQueue;


static Handle
metal_RenderPass_Allocate( const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf )
{
    DVASSERT(cmdBufCount);

    Handle                      pass_h = RenderPassPool::Alloc();
    RenderPassMetal_t*          pass   = RenderPassPool::Get( pass_h );
    MTLRenderPassDescriptor*    desc   = [MTLRenderPassDescriptor renderPassDescriptor];
    
    if( !_CurDrawable )
    {
SCOPED_NAMED_TIMING("rhi.mtl-vsync");
        _CurDrawable = [_Metal_Layer nextDrawable];
    }

    desc.colorAttachments[0].texture = _CurDrawable.texture;
    if( passConf.colorBuffer[0].texture != InvalidHandle )
        TextureMetal::SetAsRenderTarget( passConf.colorBuffer[0].texture, desc );
    
    desc.colorAttachments[0].loadAction     = (passConf.colorBuffer[0].loadAction==LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
    desc.colorAttachments[0].storeAction    = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor     = MTLClearColorMake(passConf.colorBuffer[0].clearColor[0],passConf.colorBuffer[0].clearColor[1],passConf.colorBuffer[0].clearColor[2],passConf.colorBuffer[0].clearColor[3]);

    desc.depthAttachment.texture            = _Metal_DefDepthBuf;
    desc.depthAttachment.loadAction         = (passConf.depthStencilBuffer.loadAction==LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
    desc.depthAttachment.storeAction        = (passConf.depthStencilBuffer.storeAction==STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
    desc.depthAttachment.clearDepth         = passConf.depthStencilBuffer.clearDepth;

    desc.stencilAttachment.texture          = _Metal_DefStencilBuf;
    desc.stencilAttachment.loadAction       = (passConf.depthStencilBuffer.loadAction==LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
    desc.stencilAttachment.storeAction      = (passConf.depthStencilBuffer.storeAction==STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
    desc.stencilAttachment.clearStencil     = passConf.depthStencilBuffer.clearStencil;

    if( passConf.depthStencilBuffer.texture != InvalidHandle )
        TextureMetal::SetAsDepthStencil( passConf.depthStencilBuffer.texture, desc );
    
    if( passConf.queryBuffer != InvalidHandle )
    {
        desc.visibilityResultBuffer = QueryBufferMetal::GetBuffer( passConf.queryBuffer );
    }
    
    pass->cmdBuf.resize( cmdBufCount );
    pass->priority = passConf.priority;
    
    if( cmdBufCount == 1 )
    {
        Handle                  cb_h = CommandBufferPool::Alloc();
        CommandBufferMetal_t*   cb   = CommandBufferPool::Get( cb_h );
        
        pass->desc      = desc;
        pass->encoder   = nil;
//        pass->buf       = [_Metal_DefCmdQueue commandBuffer];
        pass->buf       = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences]; 

        cb->encoder      = [pass->buf renderCommandEncoderWithDescriptor:desc];
        cb->buf          = pass->buf;
        cb->rt           = desc.colorAttachments[0].texture;
        cb->cur_ib       = InvalidHandle;
        
        pass->cmdBuf[0] = cb_h;        
        cmdBuf[0]       = cb_h;
    }
    else
    {
        pass->desc      = desc;
        pass->buf     = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
        pass->encoder = [pass->buf parallelRenderCommandEncoderWithDescriptor:desc];

        for( unsigned i=0; i!=cmdBufCount; ++i )
        {
            Handle                  cb_h = CommandBufferPool::Alloc();
            CommandBufferMetal_t*   cb   = CommandBufferPool::Get( cb_h );

            cb->encoder     = [pass->encoder renderCommandEncoder];
            cb->buf         = pass->buf;
            cb->rt          = desc.colorAttachments[0].texture;
            cb->cur_ib      = InvalidHandle;
            
            pass->cmdBuf[i] = cb_h;        
            cmdBuf[i]       = cb_h;
        }
    }

    return pass_h;
}

static void
metal_RenderPass_Begin( Handle pass_h )
{
    RenderPassMetal_t*  pass = RenderPassPool::Get( pass_h );

    _CmdQueue.push_back( pass_h );
}

static void
metal_RenderPass_End( Handle pass_h )
{
    RenderPassMetal_t*  pass = RenderPassPool::Get( pass_h );

    if( pass->cmdBuf.size() > 1 )
    {
        [pass->encoder endEncoding];
    }
}

namespace RenderPassMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Renderpass_Allocate  = &metal_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin     = &metal_RenderPass_Begin;
    dispatch->impl_Renderpass_End       = &metal_RenderPass_End;
}

}



//------------------------------------------------------------------------------

static void
metal_CommandBuffer_Begin( Handle cmdBuf )
{
    CommandBufferMetal_t*   cb    = CommandBufferPool::Get( cmdBuf );
    
    [cb->encoder setDepthStencilState:_Metal_DefDepthState];
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_End( Handle cmdBuf, Handle syncObject )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    [cb->encoder endEncoding];

    if( syncObject != InvalidHandle )
    {
        [cb->buf addCompletedHandler:^(id <MTLCommandBuffer> cmdb)
        {
            SyncObjectMetal_t*  sync = SyncObjectPool::Get( syncObject );
            
            sync->is_signaled = true;
        }];
    }
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetPipelineState( Handle cmdBuf, Handle ps, uint32 layoutUID )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    PipelineStateMetal::SetToRHI( ps, layoutUID, cb->encoder );
    StatSet::IncStat( stat_SET_PS, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetCullMode( Handle cmdBuf, CullMode mode )
{
    id<MTLRenderCommandEncoder> encoder = CommandBufferPool::Get( cmdBuf )->encoder;


    switch( mode )
    {
        case CULL_NONE :
            [encoder setCullMode:MTLCullModeNone ];
            break;
        
        case CULL_CCW :
            [encoder setFrontFacingWinding:MTLWindingClockwise ];
            [encoder setCullMode:MTLCullModeBack ];
            break;
        
        case CULL_CW :
            [encoder setFrontFacingWinding:MTLWindingClockwise ];
            [encoder setCullMode:MTLCullModeFront ];
            break;
    }
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetScissorRect( Handle cmdBuf, ScissorRect rect )
{
    CommandBufferMetal_t*       cb      = CommandBufferPool::Get( cmdBuf );
    id<MTLRenderCommandEncoder> encoder = cb->encoder;
    MTLScissorRect              rc;

    if( !(rect.x==0  &&  rect.y==0  &&  rect.width==0  &&  rect.height==0) )
    {
        rc.x      = rect.x;
        rc.y      = rect.y;
        rc.width  = rect.width;
        rc.height = rect.height;
    }
    else
    {
        rc.x      = 0;
        rc.y      = 0;
        rc.width  = cb->rt.width;
        rc.height = cb->rt.height;
    }
    
    [encoder setScissorRect:rc];
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetViewport( Handle cmdBuf, Viewport viewport )
{
    CommandBufferMetal_t*       cb      = CommandBufferPool::Get( cmdBuf );
    id<MTLRenderCommandEncoder> encoder = cb->encoder;
    MTLViewport                 vp;

    if( !(viewport.x==0  &&  viewport.y==0  &&  viewport.width==0  &&  viewport.height==0) )
    {
        vp.originX  = viewport.x;
        vp.originY  = viewport.y;
        vp.width    = viewport.width;
        vp.height   = viewport.height;
        vp.znear    = 0.0;
        vp.zfar     = 1.0;
    }
    else
    {
        vp.originX  = 0;
        vp.originY  = 0;
        vp.width    = cb->rt.width;
        vp.height   = cb->rt.height;
        vp.znear    = 0.0;
        vp.zfar     = 1.0;
    }
    
    [encoder setViewport:vp];
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    VertexBufferMetal::SetToRHI( vb, cb->encoder );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    ConstBufferMetal::SetToRHI( buffer, bufIndex, cb->encoder );
    StatSet::IncStat( stat_SET_CB, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    TextureMetal::SetToRHIVertex( tex, unitIndex, cb->encoder );
    StatSet::IncStat( stat_SET_TEX, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    cb->cur_ib = ib;
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryIndex( Handle cmdBuf, uint32 objectIndex )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );
    
    if( objectIndex != InvalidIndex )
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:objectIndex*QueryBUfferElemeentAlign];
    }
    else
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
    }
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryBuffer( Handle /*cmdBuf*/, Handle /*queryBuf*/ )
{
    // do NOTHING, since query-buffer specified in render-pass
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    ConstBufferMetal::SetToRHI( buffer, bufIndex, cb->encoder );
    StatSet::IncStat( stat_SET_CB, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    TextureMetal::SetToRHIFragment( tex, unitIndex, cb->encoder );
    StatSet::IncStat( stat_SET_TEX, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );

    DepthStencilStateMetal::SetToRHI( depthStencilState, cb->encoder );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
    CommandBufferMetal_t*   cb = CommandBufferPool::Get( cmdBuf );
    
    SamplerStateMetal::SetToRHI( samplerState, cb->encoder );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    CommandBufferMetal_t*   cb    = CommandBufferPool::Get( cmdBuf );
    MTLPrimitiveType        ptype = MTLPrimitiveTypeTriangle;
    unsigned                v_cnt = 0;
    
    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            ptype = MTLPrimitiveTypeTriangle;
            v_cnt = count * 3;
            break;
        
        case PRIMITIVE_TRIANGLESTRIP :
            ptype = MTLPrimitiveTypeTriangleStrip;
            v_cnt = 2 + count;
            break;
        
        case PRIMITIVE_LINELIST :
            ptype = MTLPrimitiveTypeLine;
            v_cnt = count * 2;
            break;
    }    

    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt];
    StatSet::IncStat( stat_DP, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 /*firstVertex*/, uint32 startIndex )
{
    CommandBufferMetal_t*   cb    = CommandBufferPool::Get( cmdBuf );
    MTLPrimitiveType        ptype = MTLPrimitiveTypeTriangle;
    unsigned                i_cnt = 0;
    id<MTLBuffer>           ib    = IndexBufferMetal::GetBuffer( cb->cur_ib );
    
    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            ptype = MTLPrimitiveTypeTriangle;
            i_cnt = count * 3;
            break;
        
        case PRIMITIVE_TRIANGLESTRIP :
            ptype = MTLPrimitiveTypeTriangleStrip;
            i_cnt = 2 + count;
            break;
        
        case PRIMITIVE_LINELIST :
            ptype = MTLPrimitiveTypeLine;
            i_cnt = count * 2;
            break;
    }    

    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:MTLIndexTypeUInt16 indexBuffer:ib indexBufferOffset:startIndex*sizeof(uint16) ];
    StatSet::IncStat( stat_DIP, 1 );
}


//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetMarker( Handle cmdBuf, const char* text )
{
    CommandBufferMetal_t*   cb  = CommandBufferPool::Get( cmdBuf );
    NSString*               txt = [[NSString alloc] initWithUTF8String:text];

    [cb->encoder insertDebugSignpost:txt];
}


//------------------------------------------------------------------------------

static Handle
metal_SyncObject_Create()
{
    Handle              handle = SyncObjectPool::Alloc();
    SyncObjectMetal_t*  sync   = SyncObjectPool::Get( handle );
    
    sync->is_signaled = false;

    return handle;
}


//------------------------------------------------------------------------------

static void
metal_SyncObject_Delete( Handle obj )
{
    SyncObjectPool::Free( obj );
}


//------------------------------------------------------------------------------

static bool
metal_SyncObject_IsSignaled( Handle obj )
{
    bool                signaled = false;
    SyncObjectMetal_t*  sync     = SyncObjectPool::Get( obj );
    
    if( sync )
        signaled = sync->is_signaled;

    return signaled;
}




//------------------------------------------------------------------------------

static void
metal_Present( Handle syncObject)
{
SCOPED_NAMED_TIMING("rhi.draw-present");

    static std::vector<RenderPassMetal_t*>    pass;

    // sort cmd-lists by priority

    pass.clear();
    for( unsigned i=0; i!=_CmdQueue.size(); ++i )
    {
        RenderPassMetal_t*  rp     = RenderPassPool::Get( _CmdQueue[i] );
        bool                do_add = true;
        
        for( std::vector<RenderPassMetal_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
        {
            if( rp->priority > (*p)->priority )
            {
                pass.insert( p, 1, rp );
                do_add = false;
                break;
            }
        }

        if( do_add )
            pass.push_back( rp );
    }
    
    if( syncObject != InvalidHandle && pass.size() && pass.back()->cmdBuf.size())
    {
        Handle                  last_cb_h = pass.back()->cmdBuf.back();
        CommandBufferMetal_t*   last_cb   = CommandBufferPool::Get( last_cb_h );
        
        [last_cb->buf addCompletedHandler:^(id <MTLCommandBuffer> cmdb)
         {
             SyncObjectMetal_t*  sync = SyncObjectPool::Get( syncObject );
             
             sync->is_signaled = true;
         }];
    }
    
    for( std::vector<RenderPassMetal_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
    {
        RenderPassMetal_t*  pass = *p;
        
        for( unsigned b=0; b!=pass->cmdBuf.size(); ++b )
        {
            Handle                  cb_h = pass->cmdBuf[b];
            
            CommandBufferPool::Free( cb_h );
        }
        
        [pass->buf presentDrawable:_CurDrawable];
        [pass->buf commit];
    }

    for( unsigned i=0; i!=_CmdQueue.size(); ++i )
        RenderPassPool::Free( _CmdQueue[i] );
    _CmdQueue.clear();

    _CurDrawable = nil;

    ConstBufferMetal::InvalidateAllInstances();
}

namespace CommandBufferMetal
{
void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_CommandBuffer_Begin                  = &metal_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End                    = &metal_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState       = &metal_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode            = &metal_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect         = &metal_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport            = &metal_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetVertexData          = &metal_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer   = &metal_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture       = &metal_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices             = &metal_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer         = &metal_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex          = &metal_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &metal_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture     = &metal_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState   = &metal_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState        = &metal_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive          = &metal_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive   = &metal_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker              = &metal_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create                    = &metal_SyncObject_Create;
    dispatch->impl_SyncObject_Delete                    = &metal_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled                = &metal_SyncObject_IsSignaled;
    
    dispatch->impl_Present                              = &metal_Present;
}
}


} // namespace rhi