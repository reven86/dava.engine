
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

class
CommandBuffer_t
{
public:

    id<MTLCommandBuffer>        buf;
    id<MTLRenderCommandEncoder> encoder;

    Handle                      cur_ib;
};

typedef Pool<CommandBuffer_t>   CommandBufferPool;

static id<CAMetalDrawable>  _CurDrawable = nil;    


namespace CommandBuffer
{

//------------------------------------------------------------------------------

Handle
Default()
{
    static Handle cb = 0;

    if( !cb )
    {
        cb = CommandBufferPool::Alloc();
        
        CommandBufferPool::Get(cb)->buf     = nil;
        CommandBufferPool::Get(cb)->encoder = nil;
    }

    return cb;
}


//------------------------------------------------------------------------------

void
Begin( Handle cmdBuf )
{
    CAMetalLayer*       layer   = (CAMetalLayer*)(GetAppViewLayer());
    CommandBuffer_t*    cb      = CommandBufferPool::Get( cmdBuf );

//    if( !_CurDrawable )
        _CurDrawable = [layer nextDrawable];

///    _Metal_DefRenderPassDescriptor.colorAttachments[0].texture = _CurDrawable.texture;
///    _Metal_DefRenderPassDescriptor.depthAttachment.texture     = _Metal_DefDepthBuf;

    MTLRenderPassDescriptor*    desc = [MTLRenderPassDescriptor renderPassDescriptor];
    
    desc.colorAttachments[0].texture        = _CurDrawable.texture;
    desc.colorAttachments[0].loadAction     = MTLLoadActionClear;
    desc.colorAttachments[0].storeAction    = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor     = MTLClearColorMake(0.3,0.3,0.6,1);

    desc.depthAttachment.texture            = _Metal_DefDepthBuf;
    desc.depthAttachment.loadAction         = MTLLoadActionClear;
    desc.depthAttachment.storeAction        = MTLStoreActionStore;
    desc.depthAttachment.clearDepth         = 1.0f;

    cb->buf     = [_Metal_DefCmdQueue commandBuffer];
//    cb->encoder = [cb->buf renderCommandEncoderWithDescriptor:_Metal_DefRenderPassDescriptor];
    cb->encoder = [cb->buf renderCommandEncoderWithDescriptor:desc];
    cb->cur_ib  = InvalidHandle;

    MTLViewport vp;

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
//    L_ASSERT(tex);
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
    CAMetalLayer*       layer = (CAMetalLayer*)(GetAppViewLayer());
    CommandBuffer_t*    cb    = Pool<CommandBuffer_t>::Get( CommandBuffer::Default() );

    [cb->buf presentDrawable:_CurDrawable];
    [cb->buf commit];
    // force sync
    [cb->buf waitUntilCompleted];
    
//    _CurDrawable = [layer nextDrawable];    
}


} // namespace rhi