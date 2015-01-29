
    #include "../RHI/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "../rhi_Base.h"
    #include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Core/Core.h"
    #include "Base/Profiler.h"

    #include "_metal.h"
    #include <QuartzCore/CAMetalLayer.h>
    #include "Platform/TemplateiOS/EAGLView.h"


namespace rhi
{

struct
RenderPass_t
{
    MTLRenderPassDescriptor*            desc;
    id<MTLCommandBuffer>                buf;
    id<MTLParallelRenderCommandEncoder> encoder;
    std::vector<Handle>                 cmdBuf;
};


struct
CommandBuffer_t
{
    id<MTLRenderCommandEncoder> encoder;

    Handle                      cur_ib;
};

typedef Pool<CommandBuffer_t>   CommandBufferPool;
typedef Pool<RenderPass_t>      RenderPassPool;

static id<CAMetalDrawable>      _CurDrawable = nil;    
static std::vector<Handle>      _CmdQueue;


namespace RenderPass
{

Handle
Allocate( const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf )
{
    DVASSERT(cmdBufCount);

    Handle                      pass_h = RenderPassPool::Alloc();
    RenderPass_t*               pass   = RenderPassPool::Get( pass_h );
    CAMetalLayer*               layer  = (CAMetalLayer*)(GetAppViewLayer());
    MTLRenderPassDescriptor*    desc   = [MTLRenderPassDescriptor renderPassDescriptor];
    

    if( !_CurDrawable )
        _CurDrawable = [layer nextDrawable];


    desc.colorAttachments[0].texture        = _CurDrawable.texture;
    desc.colorAttachments[0].loadAction     = (passConf.colorBuffer[0].loadAction==LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
    desc.colorAttachments[0].storeAction    = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor     = MTLClearColorMake(passConf.colorBuffer[0].clearColor[0],passConf.colorBuffer[0].clearColor[1],passConf.colorBuffer[0].clearColor[2],passConf.colorBuffer[0].clearColor[3]);

    desc.depthAttachment.texture            = _Metal_DefDepthBuf;
    desc.depthAttachment.loadAction         = (passConf.depthBuffer.loadAction==LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
    desc.depthAttachment.storeAction        = (passConf.depthBuffer.storeAction==STOREACTION_STORE) ? MTLStoreActionStore : MTLLoadActionDontCare;
    desc.depthAttachment.clearDepth         = passConf.depthBuffer.clearDepth;
    

    if( cmdBufCount == 1 )
    {
        Handle              cb_h = CommandBufferPool::Alloc();
        CommandBuffer_t*    cb   = CommandBufferPool::Get( cb_h );
        
        pass->desc      = desc;
        pass->encoder   = nil;
        pass->buf       = [_Metal_DefCmdQueue commandBuffer];

        cb->encoder = [pass->buf renderCommandEncoderWithDescriptor:desc];
        cb->cur_ib  = InvalidHandle;
        
        pass->cmdBuf.push_back( cb_h );        
        cmdBuf[0] = cb_h;
    }
    else
    {
        pass->desc      = desc;
        pass->buf     = [_Metal_DefCmdQueue commandBuffer];
        pass->encoder = [pass->buf parallelRenderCommandEncoderWithDescriptor:desc];
        
        for( unsigned i=0; i!=cmdBufCount; ++i )
        {
            Handle              cb_h = CommandBufferPool::Alloc();
            CommandBuffer_t*    cb   = CommandBufferPool::Get( cb_h );

            cb->encoder = [pass->encoder renderCommandEncoder];
            cb->cur_ib  = InvalidHandle;        
            
            pass->cmdBuf.push_back( cb_h );        
            cmdBuf[i] = cb_h;
        }
    }

    return pass_h;
}

void
Begin( Handle pass_h )
{
    RenderPass_t*   pass = RenderPassPool::Get( pass_h );

    _CmdQueue.push_back( pass_h );
}

void
End( Handle pass_h )
{
    RenderPass_t*   pass = RenderPassPool::Get( pass_h );

    if( pass->cmdBuf.size() > 1 )
    {
        [pass->encoder endEncoding];
    }
}

}



namespace CommandBuffer
{

//------------------------------------------------------------------------------

void
Begin( Handle cmdBuf )
{
    CommandBuffer_t* cb    = CommandBufferPool::Get( cmdBuf );
    CAMetalLayer*    layer = (CAMetalLayer*)(GetAppViewLayer());
    MTLViewport      vp;

    vp.originX  = 0;
    vp.originY  = 0;
    vp.width    = layer.bounds.size.width;
    vp.height   = layer.bounds.size.height;
    vp.znear    = 0;
    vp.zfar     = 1;

    [cb->encoder setViewport:vp];
    
    [cb->encoder setDepthStencilState:_Metal_DefDepthState];
}


//------------------------------------------------------------------------------

void
End( Handle cmdBuf )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    [cb->encoder endEncoding];
}


//------------------------------------------------------------------------------

void
Clear( Handle cmdBuf )
{
}


//------------------------------------------------------------------------------

void
SetPipelineState( Handle cmdBuf, Handle ps )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    PipelineStateMetal::SetToRHI( ps, cb->encoder );        
}


//------------------------------------------------------------------------------

void
SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    VertexBufferMetal::SetToRHI( vb, cb->encoder );
}


//------------------------------------------------------------------------------

void
SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    ConstBufferMetal::SetToRHI( buffer, bufIndex, cb->encoder );
}


//------------------------------------------------------------------------------

void
SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
}


//------------------------------------------------------------------------------

void
SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->cur_ib = ib;
}


//------------------------------------------------------------------------------

void
SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    ConstBufferMetal::SetToRHI( buffer, bufIndex, cb->encoder );
}


//------------------------------------------------------------------------------

void
SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBuffer_t*    cb = CommandBufferPool::Get( cmdBuf );

    TextureMetal::SetToRHI( tex, unitIndex, cb->encoder );
}


//------------------------------------------------------------------------------

void
SetDepthStencilState( Handle cmdBuf, const DepthStencilState& bs )
{
}


//------------------------------------------------------------------------------

void
SetSamplerState( Handle cmdBuf, const SamplerState& ss )
{
}


//------------------------------------------------------------------------------

void
DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    CommandBuffer_t*    cb    = CommandBufferPool::Get( cmdBuf );
    MTLPrimitiveType    ptype = PRIMITIVE_TRIANGLELIST;
    unsigned            v_cnt = 0;
    
    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            ptype = MTLPrimitiveTypeTriangle;
            v_cnt = count * 3;
            break;
    }    

    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt];
}


//------------------------------------------------------------------------------

void
DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    CommandBuffer_t*    cb    = CommandBufferPool::Get( cmdBuf );
    MTLPrimitiveType    ptype = PRIMITIVE_TRIANGLELIST;
    unsigned            i_cnt = 0;
    id<MTLBuffer>       ib    = IndexBufferMetal::GetBuffer( cb->cur_ib );
    
    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            ptype = MTLPrimitiveTypeTriangle;
            i_cnt = count * 3;
            break;
    }    

    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:MTLIndexTypeUInt16 indexBuffer:ib indexBufferOffset:0];
}


} // namespace CommandBuffer




//------------------------------------------------------------------------------

void
Present()
{
//    CAMetalLayer*   layer = (CAMetalLayer*)(GetAppViewLayer());
    
//    _CurDrawable = [layer nextDrawable];
    
    for( unsigned p=0; p!=_CmdQueue.size(); ++p )
    {
        RenderPass_t*   pass = RenderPassPool::Get( _CmdQueue[p] );

        for( unsigned b=0; b!=pass->cmdBuf.size(); ++b )
        {
            Handle              cb_h = pass->cmdBuf[b];
            CommandBuffer_t*    cb   = CommandBufferPool::Get( cb_h );
        

            CommandBufferPool::Free( cb_h );
        }
        
        [pass->buf presentDrawable:_CurDrawable];
        [pass->buf commit];
        // force CPU-GPU sync
        [pass->buf waitUntilCompleted];
        
        if( pass->cmdBuf.size() > 1 )
        {
        }
    }

    _CmdQueue.clear();
//    _CurDrawable = [layer nextDrawable];
    _CurDrawable = nil;
}


} // namespace rhi