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

namespace PipelineStateGLES2
{
    static int      cachedBlendEnabled  = -1;
    static GLenum   cachedBlendSrc      = (GLenum)0;
    static GLenum   cachedBlendDst      = (GLenum)0;
    static uint32   cachedProgram       = 0;
}


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
    void            InitVattr( int gl_prog, bool force_immediate=false )
                    {
                        GLCommand   cmd[16];

                        for( unsigned i=0; i!=elemCount; ++i )
                        {
                            cmd[i].func   = GLCommand::GET_ATTRIB_LOCATION;
                            cmd[i].arg[0] = gl_prog;
                            cmd[i].arg[1] = uint64_t(elem[i].name);
                        }

                        ExecGL( cmd, elemCount, force_immediate );

                        for( unsigned i=0; i!=elemCount; ++i )
                            elem[i].index = cmd[i].retval;

                        vattrInited = true;
                    }
    void            SetToRHI( uint32 firstVertex ) const
                    {
                        DVASSERT(vattrInited);

                        uint32  base                    = firstVertex * stride;
                        int     attr_used[VATTR_COUNT];

                        memset( attr_used, 0, sizeof(attr_used) );

                        struct
                        vattr_t
                        {
                            bool            enabled;
                            GLint           size;
                            GLenum          type;
                            GLboolean       normalized;
                            const GLvoid*   pointer;
                        };

                        static vattr_t  vattr[VATTR_COUNT];
                        static unsigned cur_stride = 0;
                        static bool     needInit   = true;

                        if( needInit )
                        {
//                            for( vattr_t* a=vattr,*a_end=vattr+countof(vattr); a!=a_end; ++a )
//                                a->enabled = false;
                            memset( vattr, 0, sizeof(vattr) );
                                                        
                            for( unsigned i=0; i!=VATTR_COUNT; ++i )
                                GL_CALL(glDisableVertexAttribArray( i ));

                            needInit = false;
                        }
                        
//Trace("gl.vattr-array\n");
//Trace("  base= %u  stride= %u  first_v= %u\n",base,stride,firstVertex);

                        for( unsigned i=0; i!=elemCount; ++i )
                        {
                            unsigned    idx = elem[i].index;

                            if( idx != InvalidIndex )
                                attr_used[idx] = 1;
                        }
                        
                        for( unsigned i=0; i!=countof(attr_used); ++i )
                        {
                            if( !attr_used[i] )
                            {
                                if( vattr[i].enabled )
                                {
//{SCOPED_NAMED_TIMING("gl-DisableVertexAttribArray")}
                                    GL_CALL(glDisableVertexAttribArray( i ));
                                    vattr[i].enabled = false;
                                }
                            }
                        }

                        for( unsigned i=0; i!=elemCount; ++i )
                        {
                            unsigned    idx = elem[i].index;

                            if( idx != InvalidIndex )
                            {
//Trace("[%u] count= %u  type= %u  norm= %i  stride= %u  offset= %u\n",idx,elem[i].count,elem[i].type,elem[i].normalized,stride,base+(uint8_t*)elem[i].offset);
                                if( !vattr[idx].enabled )
                                {
//{SCOPED_NAMED_TIMING("gl-EnableVertexAttribArray")}
                                    GL_CALL(glEnableVertexAttribArray( idx ));
                                    vattr[idx].enabled = true;
                                }

/*                                
                                if(     !VAttrCacheValid
                                    ||  vattr[idx].size != elem[i].count
                                    ||  vattr[idx].type != elem[i].type
                                    ||  vattr[idx].normalized != (GLboolean)(elem[i].normalized)
                                    ||  cur_stride != stride
                                    ||  vattr[idx].pointer != (const GLvoid*)(base + (uint8_t*)(elem[i].offset))
                                  )
*/                                
                                {
//{SCOPED_NAMED_TIMING("gl-VertexAttribPointer")}
                                    GL_CALL(glVertexAttribPointer( idx, elem[i].count, elem[i].type, (GLboolean)(elem[i].normalized), stride, (const GLvoid*)(base + (uint8_t*)(elem[i].offset)) ));

                                    vattr[idx].size         = elem[i].count;
                                    vattr[idx].type         = elem[i].type;
                                    vattr[idx].normalized   = (GLboolean)(elem[i].normalized);
                                    vattr[idx].pointer      = (const GLvoid*)(base + (uint8_t*)(elem[i].offset));                                    
                                }
                            }
                        }
                        VAttrCacheValid = true;

                        cur_stride = stride;                    
                    }
    static void     InvalidateVAttrCache()
                    {
                        VAttrCacheValid = false;
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

    Elem            elem[VATTR_COUNT];
    unsigned        elemCount;
    unsigned        stride;
    uint32          vattrInited:1;

    static bool     VAttrCacheValid;
};

bool    VertexDeclGLES2::VAttrCacheValid = false;


class
PipelineStateGLES2_t
{
public:
                        PipelineStateGLES2_t()
                        {}

    struct
    VertexProgGLES2
      : public ProgGLES2
    {
    public:

                                VertexProgGLES2()
                                  : ProgGLES2(PROG_VERTEX)
                                {}

        void                    SetToRHI( uint32 layoutUID, uint32 firstVertex=0 ) const
                                {
                                    if( layoutUID == VertexLayout::InvalidUID )
                                    {
                                        vdecl.SetToRHI( firstVertex );
                                    }
                                    else
                                    {
                                        for( std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v=altVdecl.begin(),v_end=altVdecl.end(); v!=v_end; ++v )
                                        {
                                            if( v->layoutUID == layoutUID )
                                            {
                                                v->vdecl.SetToRHI( firstVertex );
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
        
        mutable std::vector<vdecl_t>    altVdecl;
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

    struct
    program_t
    {
        const VertexProgGLES2*      vprog;
        const FragmentProgGLES2*    fprog;
        unsigned                    glProg;
                                    
                                    program_t() : vprog(nullptr), fprog(nullptr), glProg(0) {}
    };

    struct
    ProgramEntry
    {
        uint32              vprogSrcHash;
        VertexProgGLES2*    vprog;
        uint32              fprogSrcHash;
        FragmentProgGLES2*  fprog;
        unsigned            glProg;
        int                 refCount;
    };


    static bool                 AcquireProgram( const PipelineState::Descriptor& desc, program_t* prog );
    static void                 ReleaseProgram( const program_t& prog );

    program_t           prog;

    GLenum              blendSrc;
    GLenum              blendDst;
    bool                blendEnabled;

    GLboolean           maskR;
    GLboolean           maskG;
    GLboolean           maskB;
    GLboolean           maskA;

    
    static std::vector<ProgramEntry>    _ProgramEntry;
};

typedef ResourcePool<PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE,PipelineState::Descriptor,false>  PipelineStateGLES2Pool;
RHI_IMPL_POOL(PipelineStateGLES2_t,RESOURCE_PIPELINE_STATE,PipelineState::Descriptor,false);

std::vector<PipelineStateGLES2_t::ProgramEntry> PipelineStateGLES2_t::_ProgramEntry;


//------------------------------------------------------------------------------

bool
PipelineStateGLES2_t::AcquireProgram( const PipelineState::Descriptor& desc, PipelineStateGLES2_t::program_t* prog )
{
    bool    success = false;
    bool    doAdd = true;
    uint32  vprogSrcHash;
    uint32  fprogSrcHash;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );

    vprogSrcHash = DAVA::HashValue_N( (const char*)(&vprog_bin[0]), strlen((const char*)(&vprog_bin[0])) );
    fprogSrcHash = DAVA::HashValue_N( (const char*)(&fprog_bin[0]), strlen((const char*)(&fprog_bin[0])) );

    for( std::vector<PipelineStateGLES2_t::ProgramEntry>::iterator p=_ProgramEntry.begin(),p_end=_ProgramEntry.end(); p!=p_end; ++p )
    {
        if( p->vprogSrcHash == vprogSrcHash  &&  p->fprogSrcHash == fprogSrcHash )
        {
            prog->vprog  = p->vprog;
            prog->fprog  = p->fprog;
            prog->glProg = p->glProg;
            doAdd = false;
            success = true;
            break;
        }
    }

    if( doAdd )
    {
        ProgramEntry    entry;
        bool            vprog_valid = false;
        bool            fprog_valid = false;


        // construct vprog
        
        entry.vprog = new VertexProgGLES2();
        if( entry.vprog->Construct( (const char*)(&vprog_bin[0]) ) )
        {
            entry.vprog->vdecl.Construct( desc.vertexLayout );        
            vprog_valid = true;
        }


        // construct fprog

        entry.fprog = new FragmentProgGLES2();
        if( entry.fprog->Construct( (const char*)(&fprog_bin[0]) ) )
        {
            fprog_valid = true;
        }


        // construct pipeline-state

        if( vprog_valid  &&  fprog_valid )
        {
            GLCommand   cmd1[]      =
            {
                { GLCommand::CREATE_PROGRAM, { 0 } },
            };

            ExecGL( cmd1, countof(cmd1) );



            int         status  = 0;
            unsigned    gl_prog = cmd1[0].retval;
            GLCommand   cmd2[]      =
            {
                { GLCommand::ATTACH_SHADER, { gl_prog, entry.vprog->ShaderUid() } },
                { GLCommand::ATTACH_SHADER, { gl_prog, entry.fprog->ShaderUid() } },
                { GLCommand::LINK_PROGRAM, { gl_prog } },
                { GLCommand::GET_PROGRAM_IV, { gl_prog, GL_LINK_STATUS, (uint64_t)(&status) } },
            };

            ExecGL( cmd2, countof(cmd2) );
        
            if( status )
            {
                entry.vprog->vdecl.InitVattr( gl_prog );
                entry.vprog->GetProgParams( gl_prog );
                entry.fprog->GetProgParams( gl_prog );
            
                entry.vprog->uid    = desc.vprogUid;
                entry.vprogSrcHash  = vprogSrcHash;
                entry.fprog->uid    = desc.fprogUid;
                entry.fprogSrcHash  = fprogSrcHash;
                entry.glProg        = gl_prog;

                success = true;
            }
            else
            {
                char    info[1024];

                glGetProgramInfoLog( gl_prog, countof(info), 0, info );
                Trace( "prog-link failed:\n" );
                Trace( info );
            }                
        }

        _ProgramEntry.push_back( entry );
//Logger::Info("gl-prog cnt = %u",_ProgramEntry.size());
        prog->vprog = entry.vprog;
        prog->fprog = entry.fprog;
        prog->glProg = entry.glProg;;
    }

    return success;
}


//------------------------------------------------------------------------------

void
PipelineStateGLES2_t::ReleaseProgram( const PipelineStateGLES2_t::program_t& prog )
{
}


//------------------------------------------------------------------------------

static void
gles2_PipelineState_Delete( Handle ps )
{
    PipelineStateGLES2Pool::Free( ps );
}


//------------------------------------------------------------------------------

static Handle
gles2_PipelineState_Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = PipelineStateGLES2Pool::Alloc();;
    PipelineStateGLES2_t*       ps          = PipelineStateGLES2Pool::Get( handle );
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );
    

    if( PipelineStateGLES2_t::AcquireProgram( desc, &(ps->prog) ) )
    {
        ps->blendEnabled = desc.blending.rtBlend[0].blendEnabled;

        switch( desc.blending.rtBlend[0].colorSrc )
        {
            case BLENDOP_ZERO           : ps->blendSrc = GL_ZERO; break;
            case BLENDOP_ONE            : ps->blendSrc = GL_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendSrc = GL_SRC_ALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendSrc = GL_ONE_MINUS_SRC_ALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendSrc = GL_SRC_COLOR; break;
            case BLENDOP_DST_COLOR      : ps->blendSrc = GL_DST_COLOR; break;
        }
    
        switch( desc.blending.rtBlend[0].colorDst )
        {
            case BLENDOP_ZERO           : ps->blendDst = GL_ZERO; break;
            case BLENDOP_ONE            : ps->blendDst = GL_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendDst = GL_SRC_ALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendDst = GL_ONE_MINUS_SRC_ALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendDst = GL_SRC_COLOR; break;
            case BLENDOP_DST_COLOR      : ps->blendDst = GL_DST_COLOR; break;
        }

        ps->maskR = desc.blending.rtBlend[0].writeMask & COLORMASK_R;
        ps->maskG = desc.blending.rtBlend[0].writeMask & COLORMASK_G;
        ps->maskB = desc.blending.rtBlend[0].writeMask & COLORMASK_B;
        ps->maskA = desc.blending.rtBlend[0].writeMask & COLORMASK_A;
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
    
    return ps2->prog.vprog->InstanceConstBuffer( bufIndex );
}


//------------------------------------------------------------------------------

static Handle
gles2_PipelineState_CreateFragmentConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->prog.fprog->InstanceConstBuffer( bufIndex );
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
    
    if( ps2->prog.glProg != cachedProgram )
    {
SCOPED_NAMED_TIMING("gl-UseProgram");
//Trace("  SetProg \"%s\"\n",ps2->prog.vprog->uid.c_str());
        GL_CALL(glUseProgram( ps2->prog.glProg ));
        cachedProgram = ps2->prog.glProg;
    }

    ps2->prog.vprog->ProgGLES2::SetupTextureUnits();
    ps2->prog.fprog->ProgGLES2::SetupTextureUnits( ps2->prog.vprog->SamplerCount() );
    VertexDeclGLES2::InvalidateVAttrCache();

    
    if( ps2->blendEnabled )
    {
        if( cachedBlendEnabled != GL_TRUE )
        {
            GL_CALL(glEnable( GL_BLEND ));
            cachedBlendEnabled = GL_TRUE;
        }
        if( ps2->blendSrc != cachedBlendSrc  ||  ps2->blendDst != cachedBlendDst )
        {
            GL_CALL(glBlendFunc( ps2->blendSrc, ps2->blendDst ));
            cachedBlendSrc = ps2->blendSrc;
            cachedBlendDst = ps2->blendDst;
        }
    }
    else
    {
        if( cachedBlendEnabled != GL_FALSE )
        {
            GL_CALL(glDisable( GL_BLEND ));
            cachedBlendEnabled = GL_FALSE;
        }
    }

    static GLboolean    mask[4] = { false, false, false, false };
    
    if( ps2->maskR != mask[0]  ||  ps2->maskG != mask[1]  ||  ps2->maskB != mask[2]  ||  ps2->maskA != mask[3] )
    {
        glColorMask( ps2->maskR, ps2->maskG, ps2->maskB, ps2->maskA );
        mask[0] = ps2->maskR;
        mask[1] = ps2->maskG;
        mask[2] = ps2->maskB;
        mask[3] = ps2->maskA;
    }
}

void
SetVertexDeclToRHI( Handle ps, uint32 layoutUID, uint32 firstVertex )
{
//Trace("SetVertexDeclToRHI  layoutUID= %u\n",layoutUID);
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );

    if( layoutUID != VertexLayout::InvalidUID )
    {
        bool    do_add = true;
        
        for( std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v=ps2->prog.vprog->altVdecl.begin(),v_end=ps2->prog.vprog->altVdecl.end(); v!=v_end; ++v )
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
            vdecl.vdecl.InitVattr( ps2->prog.glProg, true );
            ps2->prog.vprog->altVdecl.push_back( vdecl );
        }
    }    

//if( layoutUID != VertexLayout::InvalidUID )
//VertexLayout::Get( layoutUID )->Dump();
    ps2->prog.vprog->SetToRHI( layoutUID, firstVertex );
}

uint32
VertexSamplerCount( Handle ps )
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get( ps );
    
    return ps2->prog.vprog->SamplerCount();
}

void
InvalidateCache()
{
    cachedBlendEnabled  = -1;
    cachedBlendSrc      = (GLenum)0;
    cachedBlendDst      = (GLenum)0;
    cachedProgram       = 0;

    VertexDeclGLES2::InvalidateVAttrCache();
}

} // namespace PipelineStateGLES2

} // namespace rhi
