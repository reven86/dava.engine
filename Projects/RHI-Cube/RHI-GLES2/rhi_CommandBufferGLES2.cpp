
    #include "rhi_CommandBufferGLES2.h"
    #include "rhi_GLES2.h"

    #include "../rhi_Base.h"
    #include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{

enum
CommandGLES2
{
    gles2_Begin,
    gles2_End,
    gles2_Clear,

    gles2_SetStreamSource,
    gles2_SetIndices,

    gles2_SetPipelineState,
    gles2_SetVertexProgConstBuffer,
    gles2_SetFragmentProgConstBuffer,
    gles2_SetFragmentTexture,

    gles2_SetBlendState,
    
    gles2_DrawPrimitive,
    gles2_DrawIndexedPrimitive,


    gles2_Nop
};



namespace CommandBuffer
{

//------------------------------------------------------------------------------

Handle
Default()
{
    static Handle cb = 0;

    if( !cb )
        cb = CommandBufferPool::Alloc();

    return cb;
}


//------------------------------------------------------------------------------

void
Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->command( gles2_Begin );
}


//------------------------------------------------------------------------------

void
End( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->command( gles2_End );
}


//------------------------------------------------------------------------------

void
Clear( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->command( gles2_Clear, 0xFF808080, nonaliased_cast<float,uint32>(1.0f) );
}


//------------------------------------------------------------------------------

void
SetPipelineState( Handle cmdBuf, Handle ps )
{
    CommandBufferPool::Get(cmdBuf)->command( gles2_SetPipelineState, ps );
}


//------------------------------------------------------------------------------

void
SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferPool::Get(cmdBuf)->command( gles2_SetStreamSource, vb, streamIndex );
}


//------------------------------------------------------------------------------

void
SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->command( gles2_SetVertexProgConstBuffer, bufIndex, buffer );
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
    CommandBufferPool::Get(cmdBuf)->command( gles2_SetIndices, ib );
}


//------------------------------------------------------------------------------

void
SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->command( gles2_SetFragmentProgConstBuffer, bufIndex, buffer );
}


//------------------------------------------------------------------------------

void
SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
//    L_ASSERT(tex);

    if( tex )
        CommandBufferPool::Get(cmdBuf)->command( gles2_SetFragmentTexture, unitIndex, tex );
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
    unsigned    v_cnt   = 0;
    int         mode    = GL_TRIANGLES;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            mode  = GL_TRIANGLES;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->command( gles2_DrawPrimitive, uint32(mode), v_cnt );
}


//------------------------------------------------------------------------------

void
DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
}


} // namespace CommandBuffer






CommandBuffer_t::CommandBuffer_t()
{
}


//------------------------------------------------------------------------------


CommandBuffer_t::~CommandBuffer_t()
{
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::begin()
{
    _cmd.clear();
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::end()
{
    _cmd.push_back( EndCmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd, uint32 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd, uint32 arg1, uint32 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3 )
{
    _cmd.resize( _cmd.size()+1+3 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+3);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4 )
{
    _cmd.resize( _cmd.size()+1+4 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+4);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5 )
{
    _cmd.resize( _cmd.size()+1+5 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+5);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
}


//------------------------------------------------------------------------------

inline void
CommandBuffer_t::command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6 )
{
    _cmd.resize( _cmd.size()+1+6 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+6);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
    b[6] = arg6; 
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::replay()
{
    Handle  cur_ps = 0;
    Handle  vp_const[MAX_CONST_BUFFER_COUNT];
    Handle  fp_const[MAX_CONST_BUFFER_COUNT];

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        vp_const[i] = InvalidHandle;
        fp_const[i] = InvalidHandle;
    }

    for( std::vector<uint32>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint32                        cmd = *c;
        std::vector<uint32>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

        switch( cmd )
        {
            case gles2_Begin :
            {
                GL_CALL(glFrontFace( GL_CW ));
                GL_CALL(glEnable( GL_CULL_FACE ));
                GL_CALL(glCullFace( GL_BACK ));
                
                GL_CALL(glEnable( GL_DEPTH_TEST ));
                GL_CALL(glDepthFunc( GL_LEQUAL ));
                GL_CALL(glDepthMask( GL_TRUE ));
            }   break;
            
            case gles2_End :
            {
                glFlush();
            }   break;
            
            case gles2_Clear :
            {
                uint32  clr (arg[0]);
                float   z   = nonaliased_cast<uint32,float>(arg[1]);
                
                glClearColor( float((clr>>0)&0xFF)/255.0f, float((clr>>8)&0xFF)/255.0f, float((clr>>16)&0xFF)/255.0f, float((clr>>24)&0xFF)/255.0f );
                glClearDepth( z );
                glClear( GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT );
                
                c += 2;    
            }   break;
            
            case gles2_SetStreamSource :
            {
                VertexBufferGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 2;
            }   break;

            case gles2_SetPipelineState :
            {
//                PipelineState_SetToRHI( (PipelineState*)(arg[0]) );
                cur_ps = (Handle)(arg[0]);
                c += 1;
            }   break;
            
            case gles2_SetVertexProgConstBuffer :
            {
                vp_const[ arg[0] ] = (Handle)(arg[1]);
                c += 2;
            }   break;

            case gles2_SetFragmentProgConstBuffer :
            {
                fp_const[ arg[0] ] = (Handle)(arg[1]);
                c += 2;
            }   break;
            
            case gles2_DrawPrimitive :
            {
                unsigned    v_cnt   = arg[1];
                int         mode    = arg[0];

                PipelineStateGLES2::SetToRHI( cur_ps );

                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( vp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( vp_const[i] );
                }
                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( fp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( fp_const[i] );
                }
                
                GL_CALL(glDrawArrays( mode, 0, v_cnt ));

                c += 2;    
            }   break;

        }
    }
}



} // namespace rhi