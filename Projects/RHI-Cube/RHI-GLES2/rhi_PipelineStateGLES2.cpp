
    #include "../rhi_Base.h"
    #include "../rhi_ShaderCache.h"
    #include "../rhi_Pool.h"

    #include "rhi_ProgGLES2.h"
    
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{

struct
VertexProgGLES2
  : public ProgGLES2
{
public:

                            VertexProgGLES2( const char* _uid )
                              : ProgGLES2(PROG_VERTEX),
                                vattrInited(false),
                                uid(_uid)
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

                            FragmentProgGLES2( const char* _uid )
                              : ProgGLES2(PROG_FRAGMENT),
                                uid(_uid)
                            {}
                            



    DAVA::FastName  uid;
};


class
PipelineStateGLES2_t
{
public:
                        PipelineStateGLES2_t()
                         : vprog(0),
                            fprog(0),
                            prog(0)
                        {}

    VertexProgGLES2*    vprog;
    FragmentProgGLES2*  fprog;
    unsigned            prog;
};

typedef Pool<PipelineStateGLES2_t>  PipelineStateGLES2Pool;


namespace PipelineState
{

Handle
Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = InvalidHandle;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );

    VertexProgGLES2*    vprog = new VertexProgGLES2( desc.vprogUid.c_str() );
    FragmentProgGLES2*  fprog = new FragmentProgGLES2( desc.fprogUid.c_str() );

    DVASSERT(vprog);
    DVASSERT(fprog);
    

    // construct vprog

    if( vprog->Construct( (const char*)(&vprog_bin[0]) ) )
    {
        for( unsigned i=0; i!=desc.vertexLayout.ElementCount(); ++i )
        {
            switch( desc.vertexLayout.ElementDataType(i) )
            {
                case VDT_FLOAT :
                {
                    vprog->elem[i].type       = GL_FLOAT;
                    vprog->elem[i].normalized = GL_FALSE;
                }   break;
            }

            switch( desc.vertexLayout.ElementSemantics(i) )
            {
                case VS_POSITION : vprog->elem[i].name = "attr_position"; break;
                case VS_NORMAL   : vprog->elem[i].name = "attr_normal"; break;
                case VS_TEXCOORD : vprog->elem[i].name = "attr_texcoord"; break;
            }
            
            vprog->elem[i].count  = desc.vertexLayout.ElementDataCount( i );
            vprog->elem[i].offset = (void*)(desc.vertexLayout.ElementOffset( i ));
            vprog->elem[i].index  = InvalidIndex;
        }

        vprog->elemCount   = desc.vertexLayout.ElementCount();
        vprog->stride      = desc.vertexLayout.Stride();
        vprog->vattrInited = false;
    }
    else
    {
        delete vprog;
        vprog = 0;
    }


    // construct fprog

    if( fprog->Construct( (const char*)(&fprog_bin[0]) ) )
    {
    }
    else
    {
        delete fprog;
        fprog = 0;
    }


    // construct pipeline-state

    if( vprog  &&  fprog )
    {
        int         status  = 0;
        unsigned    gl_prog = glCreateProgram();

        GL_CALL(glAttachShader( gl_prog, vprog->ShaderUid() ));
        GL_CALL(glAttachShader( gl_prog, fprog->ShaderUid() ));
        
        GL_CALL(glLinkProgram( gl_prog ));
        GL_CALL(glGetProgramiv( gl_prog, GL_LINK_STATUS, &status ));

        if( status )
        {
            for( unsigned i=0; i!=vprog->elemCount; ++i )
            {
                vprog->elem[i].index = glGetAttribLocation( gl_prog, vprog->elem[i].name );
            }
            vprog->vattrInited = true;
            vprog->GetProgParams( gl_prog );
            fprog->GetProgParams( gl_prog );
            
            handle = PipelineStateGLES2Pool::Alloc();

            PipelineStateGLES2_t*   ps = PipelineStateGLES2Pool::Get( handle );

            ps->vprog = vprog;
            ps->fprog = fprog;
            ps->prog  = gl_prog;
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
    
    return ps2->vprog->InstanceConstBuffer( bufIndex );
}


Handle
CreateFProgConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->fprog->InstanceConstBuffer( bufIndex );
}


} // // namespace PipelineState


namespace PipelineStateGLES2
{

void
SetToRHI( Handle ps )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );

    DVASSERT(ps2);
    GL_CALL(glUseProgram( ps2->prog ));
    ps2->vprog->SetToRHI();
}

} // namespace PipelineStateGLES2

} // namespace rhi
