
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
    
    GLenum  stencilFailOp;
    GLenum  depthFailOp;
    GLenum  depthStencilPassOp;
    GLenum  stencilFunc;
    uint32  stencilWriteMask;
    uint32  stencilReadMask;
    uint32  stencilRefValue;
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
        case CML_GREATEREQUAL   : f = GL_GEQUAL; break;
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
    
    state->stencilFailOp        = _StencilOp( StencilOperation(desc.stencilFailOperation) );
    state->depthFailOp          = _StencilOp( StencilOperation(desc.depthFailOperation) );
    state->depthStencilPassOp   = _StencilOp( StencilOperation(desc.depthStencilPassOperation) );
    state->stencilFunc          = _CmpFunc( CmpFunc(desc.stencilFunc) );
    state->stencilReadMask      = desc.stencilReadMask;
    state->stencilWriteMask     = desc.stencilWriteMask;
    state->stencilRefValue      = desc.stencilRefValue;

    
    
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

    
    glStencilOp( state->stencilFailOp, state->depthFailOp, state->depthStencilPassOp );
    glStencilFunc( state->stencilFunc, state->stencilRefValue, state->stencilReadMask );
    glStencilMask( state->stencilWriteMask );
}

}



//==============================================================================
} // namespace rhi

