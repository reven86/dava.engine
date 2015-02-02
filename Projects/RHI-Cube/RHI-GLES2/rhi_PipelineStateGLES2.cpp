
    #include "../rhi_Base.h"
    #include "../RHI/rhi_ShaderCache.h"
    #include "../RHI/rhi_Pool.h"

    #include "rhi_ProgGLES2.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{


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
                                  : ProgGLES2(PROG_VERTEX),
                                    vattrInited(false)
                                {}

        void                    SetToRHI()
                                {
                                    DVASSERT(vattrInited);

                                    ProgGLES2::SetToRHI();

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
            const char* name;
            unsigned    index;
        
            unsigned    type;
            unsigned    count;
            int         normalized;
            void*       offset;
        };

        Elem            elem[16];
        unsigned        elemCount;
        unsigned        stride;
        unsigned        vattrInited:1;

        DAVA::FastName  uid;
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
};

typedef Pool<PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE>  PipelineStateGLES2Pool;
RHI_IMPL_POOL(PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE);


namespace PipelineState
{

Handle
Create( const PipelineState::Descriptor& desc )
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
        for( unsigned i=0; i!=desc.vertexLayout.ElementCount(); ++i )
        {
            switch( desc.vertexLayout.ElementDataType(i) )
            {
                case VDT_FLOAT :
                {
                    ps->vprog.elem[i].type       = GL_FLOAT;
                    ps->vprog.elem[i].normalized = GL_FALSE;
                }   break;
            }

            switch( desc.vertexLayout.ElementSemantics(i) )
            {
                case VS_POSITION : ps->vprog.elem[i].name = "attr_position"; break;
                case VS_NORMAL   : ps->vprog.elem[i].name = "attr_normal"; break;
                case VS_TEXCOORD : ps->vprog.elem[i].name = "attr_texcoord"; break;
            }
            
            ps->vprog.elem[i].count  = desc.vertexLayout.ElementDataCount( i );
            ps->vprog.elem[i].offset = (void*)(desc.vertexLayout.ElementOffset( i ));
            ps->vprog.elem[i].index  = InvalidIndex;
        }

        ps->vprog.elemCount   = desc.vertexLayout.ElementCount();
        ps->vprog.stride      = desc.vertexLayout.Stride();
        ps->vprog.vattrInited = false;        
        vprog_valid           = true;
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
            for( unsigned i=0; i!=ps->vprog.elemCount; ++i )
            {
                ps->vprog.elem[i].index = glGetAttribLocation( gl_prog, ps->vprog.elem[i].name );
            }
            ps->vprog.vattrInited = true;
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
    }    
    return handle;
} 


Handle
CreateVProgConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->vprog.InstanceConstBuffer( bufIndex );
}


Handle
CreateFProgConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->fprog.InstanceConstBuffer( bufIndex );
}


} // // namespace PipelineState


namespace PipelineStateGLES2
{

void
SetToRHI( Handle ps )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );

    DVASSERT(ps2);
    GL_CALL(glUseProgram( ps2->glProg ));
    ps2->vprog.SetToRHI();
}

} // namespace PipelineStateGLES2

} // namespace rhi
