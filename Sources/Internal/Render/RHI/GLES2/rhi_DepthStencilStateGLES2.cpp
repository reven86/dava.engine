
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{
//==============================================================================

struct
DepthStencilStateGLES2_t
{
    uint32  depthTestEnabled:1;
    GLuint  depthMask;
    GLenum  depthFunc;
    
    uint32  stencilEnabled:1;
    uint32  stencilSeparate:1;
    struct    
    {
    GLenum  failOp;
    GLenum  depthFailOp;
    GLenum  depthStencilPassOp;
    GLenum  func;
    uint32  writeMask;
    uint32  readMask;
    uint32  refValue;
    }       stencilFront, stencilBack;
};

typedef Pool<DepthStencilStateGLES2_t,RESOURCE_DEPTHSTENCIL_STATE>  DepthStencilStateGLES2Pool;
RHI_IMPL_POOL(DepthStencilStateGLES2_t,RESOURCE_DEPTHSTENCIL_STATE);

    
//------------------------------------------------------------------------------

static GLenum
_CmpFunc( CmpFunc func )
{
    GLenum  f = GL_ALWAYS;
    
    switch( func )
    {
        case CMP_NEVER          : f = GL_NEVER; break;
        case CMP_LESS           : f = GL_LESS; break;
        case CMP_EQUAL          : f = GL_EQUAL; break;
        case CMP_LESSEQUAL      : f = GL_LEQUAL; break;
        case CMP_GREATER        : f = GL_GREATER; break;
        case CMP_NOTEQUAL       : f = GL_NOTEQUAL; break;
        case CMP_GREATEREQUAL   : f = GL_GEQUAL; break;
        case CMP_ALWAYS         : f = GL_ALWAYS; break;
    }
    
    return f;
}


//------------------------------------------------------------------------------

static GLenum
_StencilOp( StencilOperation op )
{
    GLenum  s = GL_KEEP;
    
    switch( op )
    {
        case STENCILOP_KEEP             : s = GL_KEEP; break;
        case STENCILOP_ZERO             : s = GL_ZERO; break;
        case STENCILOP_REPLACE          : s = GL_REPLACE; break;
        case STENCILOP_INVERT           : s = GL_INVERT; break;
        case STENCILOP_INCREMENT_CLAMP  : s = GL_INCR; break;
        case STENCILOP_DECREMENT_CLAMP  : s = GL_DECR; break;
        case STENCILOP_INCREMENT_WRAP   : s = GL_INCR_WRAP; break;
        case STENCILOP_DECREMENT_WRAP   : s = GL_DECR_WRAP; break;
    }
    
    return s;
}

    
//==============================================================================

static Handle
gles2_DepthStencilState_Create( const DepthStencilState::Descriptor& desc )
{
    Handle                      handle = DepthStencilStateGLES2Pool::Alloc();
    DepthStencilStateGLES2_t*   state  = DepthStencilStateGLES2Pool::Get( handle );
    
    state->depthTestEnabled     = desc.depthTestEnabled;
    state->depthMask            = (desc.depthWriteEnabled)  ? GL_TRUE  : GL_FALSE;
    state->depthFunc            = _CmpFunc( CmpFunc(desc.depthFunc) );
    
    state->stencilEnabled                   = !(desc.stencilFront.func == CMP_ALWAYS  &&  desc.stencilFront.readMask == 0xFF  && desc.stencilFront.writeMask == 0xFF);
    state->stencilSeparate                  = desc.stencilTwoSided;
    
    state->stencilFront.failOp              = _StencilOp( StencilOperation(desc.stencilFront.failOperation) );
    state->stencilFront.depthFailOp         = _StencilOp( StencilOperation(desc.stencilFront.depthFailOperation) );
    state->stencilFront.depthStencilPassOp  = _StencilOp( StencilOperation(desc.stencilFront.depthStencilPassOperation) );
    state->stencilFront.func                = _CmpFunc( CmpFunc(desc.stencilFront.func) );
    state->stencilFront.readMask            = desc.stencilFront.readMask;
    state->stencilFront.writeMask           = desc.stencilFront.writeMask;
    state->stencilFront.refValue            = desc.stencilFront.refValue;

    state->stencilBack.failOp               = _StencilOp( StencilOperation(desc.stencilBack.failOperation) );
    state->stencilBack.depthFailOp          = _StencilOp( StencilOperation(desc.stencilBack.depthFailOperation) );
    state->stencilBack.depthStencilPassOp   = _StencilOp( StencilOperation(desc.stencilBack.depthStencilPassOperation) );
    state->stencilBack.func                 = _CmpFunc( CmpFunc(desc.stencilBack.func) );
    state->stencilBack.readMask             = desc.stencilBack.readMask;
    state->stencilBack.writeMask            = desc.stencilBack.writeMask;
    state->stencilBack.refValue             = desc.stencilBack.refValue;
    
    
    return handle;
}


//------------------------------------------------------------------------------

void
gles2_DepthStencilState_Delete( Handle state )
{
    DepthStencilStateGLES2Pool::Free( state );
}

                                                                                
//==============================================================================

namespace DepthStencilStateGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_DepthStencilState_Create = &gles2_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &gles2_DepthStencilState_Delete;
}

void
SetToRHI( Handle hstate )
{
    DepthStencilStateGLES2_t*   state  = DepthStencilStateGLES2Pool::Get( hstate );
    
    if( state->depthTestEnabled )
    {
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( state->depthFunc );
    }
    else
    {
        glDisable( GL_DEPTH_TEST );
    }
    GL_CALL(glDepthMask( state->depthMask ));

    
    if( state->stencilEnabled  )
    {
        glEnable( GL_STENCIL_TEST );
        
        if( state->stencilSeparate )
        {
            glStencilOpSeparate( GL_FRONT, state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp );
            glStencilFuncSeparate( GL_FRONT, state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask );
            glStencilMaskSeparate( GL_FRONT, state->stencilFront.writeMask );

            glStencilOpSeparate( GL_BACK, state->stencilBack.failOp, state->stencilBack.depthFailOp, state->stencilBack.depthStencilPassOp );
            glStencilFuncSeparate( GL_BACK, state->stencilBack.func, state->stencilBack.refValue, state->stencilBack.readMask );
            glStencilMaskSeparate( GL_BACK, state->stencilBack.writeMask );
        }
        else
        {
            glStencilOp( state->stencilFront.failOp, state->stencilFront.depthFailOp, state->stencilFront.depthStencilPassOp );
            glStencilFunc( state->stencilFront.func, state->stencilFront.refValue, state->stencilFront.readMask );
            glStencilMask( state->stencilFront.writeMask );
        }
    }
    else
    {
        glDisable( GL_STENCIL_TEST );
    }
}

}



//==============================================================================
} // namespace rhi

