
    #include "../Common/rhi_Private.h"
    #include "../rhi_ShaderCache.h"
    #include "../Common/rhi_Pool.h"

    #include "rhi_ProgGLES2.h"
    #include "rhi_GLES2.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Debug/Profiler.h"

    #include "_gl.h"


namespace rhi
{

struct
VertexDeclGLES2
{
                    VertexDeclGLES2()
                      : elemCount(0),
                        stride(0),
                        vattrInited(false)
                    {}

    void            Construct( const VertexLayout& layout )
                    {
                        elemCount = 0;

                        for( unsigned i=0; i!=layout.ElementCount(); ++i )
                        {
                            if( layout.ElementSemantics(i) == VS_PAD )
                                continue;

                            switch( layout.ElementDataType(i) )
                            {
                                case VDT_FLOAT :
                                {
                                    elem[elemCount].type       = GL_FLOAT;
                                    elem[elemCount].normalized = GL_FALSE;
                                }   break;
/*
                                case VDT_HALF :
                                {
                                    elem[elemCount].type       = GL_HALF_FLOAT;
                                    elem[elemCount].normalized = GL_FALSE;
                                }   break;
*/                                
                                case VDT_INT16N :
                                {
                                    elem[elemCount].type       = GL_SHORT;
                                    elem[elemCount].normalized = GL_TRUE;
                                }   break;
                                
                                case VDT_UINT8N :
                                {
                                    elem[elemCount].type       = GL_UNSIGNED_BYTE;
                                    elem[elemCount].normalized = GL_TRUE;
                                }   break;
                                
                                case VDT_INT8N :
                                {
                                    elem[elemCount].type       = GL_BYTE;
                                    elem[elemCount].normalized = GL_TRUE;
                                }   break;
                                
                                case VDT_UINT8 :
                                {
                                    elem[elemCount].type       = GL_UNSIGNED_BYTE;
                                    elem[elemCount].normalized = GL_FALSE;
                                }   break;
                                    
                                default :
                                {}
                            }

                            switch( layout.ElementSemantics(i) )
                            {
                                case VS_POSITION    : strcpy( elem[elemCount].name, "attr_position" ); break;
                                case VS_NORMAL      : strcpy( elem[elemCount].name, "attr_normal" ); break;
                                case VS_TEXCOORD    : sprintf( elem[elemCount].name, "attr_texcoord%u", layout.ElementSemanticsIndex(i) ); break;
                                case VS_COLOR       : sprintf( elem[elemCount].name, "attr_color%u", layout.ElementSemanticsIndex(i) ); break;
                                case VS_TANGENT     : strcpy( elem[elemCount].name, "attr_tangent" ); break;
                                case VS_BINORMAL    : strcpy( elem[elemCount].name, "attr_binormal" ); break;
                                case VS_BLENDWEIGHT : strcpy( elem[elemCount].name, "attr_blendweight" ); break;
                                case VS_BLENDINDEX  : strcpy( elem[elemCount].name, "attr_blendindex" ); break;
                                    
                                default             : strcpy( elem[elemCount].name, "<unsupported>" );
                            }
                            
                            elem[elemCount].count  = layout.ElementDataCount( i );
                            elem[elemCount].offset = (void*)(uint64(layout.ElementOffset( i )));
                            elem[elemCount].index  = InvalidIndex;
                            
                            ++elemCount;
                        }
                        
                        stride      = layout.Stride();
                        vattrInited = false;
                    }
    void            InitVattr( int gl_prog )
                    {
                        for( unsigned i=0; i!=elemCount; ++i )
                        {
                            elem[i].index = glGetAttribLocation( gl_prog, elem[i].name );
                        }
                        vattrInited = true;
                    }
    void            SetToRHI()
                    {
                        DVASSERT(vattrInited);
                        
                        for( unsigned i=0; i!=elemCount; ++i )
                        {
                            unsigned    idx = elem[i].index;

                            if( idx != InvalidIndex )
                            {
                                GL_CALL(glEnableVertexAttribArray( idx ));
                                GL_CALL(glVertexAttribPointer( idx, elem[i].count, elem[i].type, (GLboolean)(elem[i].normalized), stride, elem[i].offset ));
                            }
                        }
                    }

    struct
    Elem
    {
        char        name[32];
        unsigned    index;
    
        unsigned    type;
        unsigned    count;
        int         normalized;
        void*       offset;
    };

    Elem            elem[16];
    unsigned        elemCount;
    unsigned        stride;
    uint32          vattrInited:1;
};


class
PipelineStateGLES2_t
{
public:
                        PipelineStateGLES2_t()
                          : glProg(0)
                        {}

    struct
    VertexProgGLES2
      : public ProgGLES2
    {
    public:

                                VertexProgGLES2()
                                  : ProgGLES2(PROG_VERTEX)
                                {}

        void                    SetToRHI( uint32 layoutUID )
                                {
                                    if( layoutUID == VertexLayout::InvalidUID )
                                    {
                                        vdecl.SetToRHI();
                                    }
                                    else
                                    {
                                        for( std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v=altVdecl.begin(),v_end=altVdecl.end(); v!=v_end; ++v )
                                        {
                                            if( v->layoutUID == layoutUID )
                                            {
                                                v->vdecl.SetToRHI();
                                                break;
                                            }
                                        }
                                    }
                                }

        VertexDeclGLES2 vdecl;
        DAVA::FastName  uid;
        
        struct
        vdecl_t
        {
            VertexDeclGLES2 vdecl;
            uint32          layoutUID;
        };
        
        std::vector<vdecl_t>    altVdecl;
    };

    struct
    FragmentProgGLES2
      : public ProgGLES2
    {
    public:

                                FragmentProgGLES2()
                                  : ProgGLES2(PROG_FRAGMENT)
                                {}
                            



        DAVA::FastName  uid;
    };


    VertexProgGLES2     vprog;
    FragmentProgGLES2   fprog;
    unsigned            glProg;

    GLenum              blendSrc;
    GLenum              blendDst;
    bool                blendEnabled;
};

typedef Pool<PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE>  PipelineStateGLES2Pool;
RHI_IMPL_POOL(PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE);


//------------------------------------------------------------------------------

static void
gles2_PipelineState_Delete( Handle ps )
{
}


//------------------------------------------------------------------------------

static Handle
gles2_PipelineState_Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = PipelineStateGLES2Pool::Alloc();;
    PipelineStateGLES2_t*       ps          = PipelineStateGLES2Pool::Get( handle );
    bool                        vprog_valid = false;
    bool                        fprog_valid = false;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );


    // construct vprog
    
    if( ps->vprog.Construct( (const char*)(&vprog_bin[0]) ) )
    {
        ps->vprog.vdecl.Construct( desc.vertexLayout );        
        vprog_valid = true;
    }


    // construct fprog

    if( ps->fprog.Construct( (const char*)(&fprog_bin[0]) ) )
    {
        fprog_valid = true;
    }


    // construct pipeline-state

    if( vprog_valid  &&  fprog_valid )
    {
        int         status  = 0;
        unsigned    gl_prog = glCreateProgram();

        GL_CALL(glAttachShader( gl_prog, ps->vprog.ShaderUid() ));
        GL_CALL(glAttachShader( gl_prog, ps->fprog.ShaderUid() ));
        
        GL_CALL(glLinkProgram( gl_prog ));
        GL_CALL(glGetProgramiv( gl_prog, GL_LINK_STATUS, &status ));

        if( status )
        {
            ps->vprog.vdecl.InitVattr( gl_prog );
            ps->vprog.GetProgParams( gl_prog );
            ps->fprog.GetProgParams( gl_prog );
            
            ps->vprog.uid = desc.vprogUid;
            ps->fprog.uid = desc.fprogUid;
            ps->glProg    = gl_prog;
        }
        else
        {
            char    info[1024];

            glGetProgramInfoLog( gl_prog, countof(info), 0, info );
            Logger::Error( "prog-link failed:" );
            Logger::Info( info );
        }                
    
        ps->blendEnabled = desc.blending.rtBlend[0].blendEnabled;

        switch( desc.blending.rtBlend[0].colorSrc )
        {
            case BLENDOP_ZERO           : ps->blendSrc = GL_ZERO; break;
            case BLENDOP_ONE            : ps->blendSrc = GL_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendSrc = GL_SRC_ALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendSrc = GL_ONE_MINUS_SRC_ALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendSrc = GL_SRC_COLOR; break;
        }
    
        switch( desc.blending.rtBlend[0].colorDst )
        {
            case BLENDOP_ZERO           : ps->blendDst = GL_ZERO; break;
            case BLENDOP_ONE            : ps->blendDst = GL_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendDst = GL_SRC_ALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendDst = GL_ONE_MINUS_SRC_ALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendDst = GL_SRC_COLOR; break;
        }
    }
    else
    {
        PipelineStateGLES2Pool::Free( handle );
        handle = InvalidHandle;
    }

    
    return handle;
} 


//------------------------------------------------------------------------------

static Handle
gles2_PipelineState_CreateVertexConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->vprog.InstanceConstBuffer( bufIndex );
}


//------------------------------------------------------------------------------

static Handle
gles2_PipelineState_CreateFragmentConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->fprog.InstanceConstBuffer( bufIndex );
}


namespace PipelineStateGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_PipelineState_Create                     = &gles2_PipelineState_Create;
    dispatch->impl_PipelineState_Delete                     = &gles2_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer    = &gles2_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer  = &gles2_PipelineState_CreateFragmentConstBuffer;
}

void
SetToRHI( Handle ps, uint32 layoutUID )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );

    DVASSERT(ps2);
    
    GL_CALL(glUseProgram( ps2->glProg ));
    ps2->vprog.ProgGLES2::SetupTextureUnits();
    ps2->fprog.ProgGLES2::SetupTextureUnits( ps2->vprog.SamplerCount() );
    
    if( ps2->blendEnabled )
    {
        GL_CALL(glEnable( GL_BLEND ));
        GL_CALL(glBlendFunc( ps2->blendSrc, ps2->blendDst ));
    }
    else
    {
        GL_CALL(glDisable( GL_BLEND ));
    }
}

void
SetVertexDeclToRHI( Handle ps, uint32 layoutUID )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );

    if( layoutUID != VertexLayout::InvalidUID )
    {
        bool    do_add = true;
        
        for( std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v=ps2->vprog.altVdecl.begin(),v_end=ps2->vprog.altVdecl.end(); v!=v_end; ++v )
        {
            if( v->layoutUID == layoutUID )
            {
                do_add = false;
                break;
            }
        }

        if( do_add )
        {
            const VertexLayout*                             layout = VertexLayout::Get( layoutUID );
            PipelineStateGLES2_t::VertexProgGLES2::vdecl_t  vdecl;
            
            vdecl.layoutUID = layoutUID;
            vdecl.vdecl.Construct( *layout );
            vdecl.vdecl.InitVattr( ps2->glProg );
            ps2->vprog.altVdecl.push_back( vdecl );
        }
    }    
    
    ps2->vprog.SetToRHI( layoutUID );
}

uint32
VertexSamplerCount( Handle ps )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->vprog.SamplerCount();
}

} // namespace PipelineStateGLES2

} // namespace rhi
