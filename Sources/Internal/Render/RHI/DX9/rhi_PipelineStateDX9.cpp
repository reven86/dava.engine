//  externals:

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_RingBuffer.h"
    #include "../rhi_ShaderCache.h"
    #include "rhi_DX9.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    using DAVA::uint32;
    using DAVA::uint16;
    using DAVA::uint8;

    #include "_dx9.h"
    #include <D3DX9Shader.h>

    #include <vector>


namespace rhi
{

struct
VDeclDX9
{
    VertexLayout                        layout;
    IDirect3DVertexDeclaration9*        vdecl9;

    static IDirect3DVertexDeclaration9* Get( const VertexLayout& layout );


private:

    static std::vector<VDeclDX9>    _VDecl;
};
std::vector<VDeclDX9>   VDeclDX9::_VDecl;

static RingBuffer       _DefConstRingBuf;


static void
DumpShaderText( const char* code, unsigned code_sz )
{
    char        src[64*1024];
    char*       src_line[1024];
    unsigned    line_cnt        = 0;
    
    if( code_sz < sizeof(src) )
    {
        memcpy( src, code, code_sz );
        src[code_sz] = '\0';
        memset( src_line, 0, sizeof(src_line) );

        src_line[line_cnt++] = src;
        for( char* s=src; *s; )
        {
            if( *s == '\n' )
            {
                *s = 0;                
                ++s;

                while( *s  &&  (/**s == '\n'  ||  */*s == '\r') )
                {
                    *s = 0;
                    ++s;
                }

                if( !(*s) )
                    break;            
                
                src_line[line_cnt] = s;
                ++line_cnt;
            }
            else if( *s == '\r' )
            {
                *s = ' ';
            }
            else
            {
                ++s;
            }
        }
    
        for( unsigned i=0; i!=line_cnt; ++i )
        {
            Logger::Info( "%4u |  %s", 1+i, src_line[i] );
        }
    }
    else
    {
        Logger::Info( code );
    }
}


//------------------------------------------------------------------------------

IDirect3DVertexDeclaration9*
VDeclDX9::Get( const VertexLayout& layout )
{
    IDirect3DVertexDeclaration9*    vdecl = nullptr;

    for( std::vector<VDeclDX9>::const_iterator v=_VDecl.begin(),v_end=_VDecl.end(); v!=v_end; ++v )
    {
        if( v->layout == layout )
        {
            vdecl = v->vdecl9;
            break;
        }
    }

    if( !vdecl )
    {
        IDirect3DVertexDeclaration9*    vd9         = nullptr;
        D3DVERTEXELEMENT9               elem[32];
        uint32                          elemCount   = 0;
        HRESULT                         hr;

        DVASSERT(layout.ElementCount() < countof(elem));
        for( unsigned i=0; i!=layout.ElementCount(); ++i )
        {
            if( layout.ElementSemantics(i) == VS_PAD )
                continue;


            elem[elemCount].Stream      = 0;
            elem[elemCount].Offset      = (WORD)(layout.ElementOffset( i ));
            elem[elemCount].Method      = D3DDECLMETHOD_DEFAULT;
            elem[elemCount].UsageIndex  = layout.ElementSemanticsIndex( i );

            switch( layout.ElementSemantics(i) )
            {
                case VS_POSITION    : elem[elemCount].Usage = D3DDECLUSAGE_POSITION; break;
                case VS_NORMAL      : elem[elemCount].Usage = D3DDECLUSAGE_NORMAL; break;
                case VS_COLOR       : elem[elemCount].Usage = D3DDECLUSAGE_COLOR; break;
                case VS_TEXCOORD    : elem[elemCount].Usage = D3DDECLUSAGE_TEXCOORD; break;
                case VS_TANGENT     : elem[elemCount].Usage = D3DDECLUSAGE_TANGENT; break;
                case VS_BINORMAL    : elem[elemCount].Usage = D3DDECLUSAGE_BINORMAL; break;
                case VS_BLENDWEIGHT : elem[elemCount].Usage = D3DDECLUSAGE_BLENDWEIGHT; break;
                case VS_BLENDINDEX  : elem[elemCount].Usage = D3DDECLUSAGE_BLENDINDICES; break;
            }

            switch( layout.ElementDataType(i) )
            {
                case VDT_FLOAT :
                {
                    switch( layout.ElementDataCount(i) )
                    {
                        case 4 : elem[elemCount].Type = D3DDECLTYPE_FLOAT4; break;
                        case 3 : elem[elemCount].Type = D3DDECLTYPE_FLOAT3; break;
                        case 2 : elem[elemCount].Type = D3DDECLTYPE_FLOAT2; break;
                    }
                }   break;
            }

            if( layout.ElementSemantics(i) == VS_COLOR )
            {
                elem[elemCount].Type = D3DDECLTYPE_D3DCOLOR;
            }

            ++elemCount;
        }
        elem[elemCount].Stream     = 0xFF;
        elem[elemCount].Offset     = 0;
        elem[elemCount].Type       = D3DDECLTYPE_UNUSED;
        elem[elemCount].Method     = 0;
        elem[elemCount].Usage      = 0;
        elem[elemCount].UsageIndex = 0;
        

        hr = _D3D9_Device->CreateVertexDeclaration( elem, &vd9 );

        if( SUCCEEDED(hr) )
        {
            VDeclDX9    vd;

            vd.vdecl9 = vd9;
            vd.layout = layout;
            _VDecl.push_back( vd );

            vdecl = vd9;

        }
        else
        {
            Logger::Error( "FAILED to create vertex-decl:\n%s\n", D3D9ErrorText(hr) );
            layout.Dump();
        }
    }

    return vdecl;
}


//==============================================================================

class
PipelineStateDX9_t
{
public:
                    PipelineStateDX9_t()
                    {}

    class
    ConstBuf
    {
    public:
                    ConstBuf();
                    ~ConstBuf();
    
        void        Construct( ProgType type, unsigned reg_i, unsigned reg_count );
        void        Destroy();

        unsigned    ConstCount() const;
        const void* InstData() const;
        void        InvalidateInst();

        bool        SetConst( unsigned const_i, unsigned count, const float* data );
        bool        SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount );
        void        SetToRHI( const void* inst_data ) const;


    private:

        ProgType        progType;
        float*          value;
        mutable float*  inst;
        unsigned        reg;
        unsigned        regCount;
    };

    struct
    VertexProgDX9
    {
    public:

                                        VertexProgDX9()
                                          : stride(0),
                                            codeSiize(0),
                                            code(nullptr),
                                            vs9(nullptr),
                                            vdecl9(nullptr)
                                        {
                                        }

        bool                            Construct( const void* code, unsigned code_sz, const VertexLayout& vdecl );
        Handle                          CreateConstBuffer( unsigned buf_i );
        void                            SetToRHI( uint32 layoutUID );
        
        struct
        vdecl_t
        {
            uint32                          layoutUID;
            IDirect3DVertexDeclaration9*    vdecl;
        };
        
        unsigned                        stride;
        unsigned                        codeSiize;
        void*                           code;
        IDirect3DVertexShader9*         vs9;
        IDirect3DVertexDeclaration9*    vdecl9;    // ref-only
        std::vector<vdecl_t>            altVdecl9;
        unsigned                        cbufReg[MAX_CONST_BUFFER_COUNT];
        unsigned                        cbufCount[MAX_CONST_BUFFER_COUNT];
    };

    struct
    FragmentProgDX9
    {
    public:

                                FragmentProgDX9()
                                  : codeSize(0),
                                    code(nullptr),
                                    ps9(nullptr)
                                {
                                }

        bool                    Construct( const void* code, unsigned code_sz );
        Handle                  CreateConstBuffer( unsigned buf_i );
        void                    SetToRHI();

        unsigned                codeSize;
        void*                   code;
        IDirect3DPixelShader9*  ps9;
        unsigned                cbufReg[MAX_CONST_BUFFER_COUNT];
        unsigned                cbufCount[MAX_CONST_BUFFER_COUNT];
    };

    VertexProgDX9       vprog;
    FragmentProgDX9     fprog;
    
    DWORD               blendSrc;
    DWORD               blendDst;
    bool                blendEnabled;
};

typedef Pool<PipelineStateDX9_t,RESOURCE_PIPELINE_STATE>            PipelineStateDX9Pool;
typedef Pool<PipelineStateDX9_t::ConstBuf,RESOURCE_CONST_BUFFER>    ConstBufDX9Pool;

RHI_IMPL_POOL(PipelineStateDX9_t,RESOURCE_PIPELINE_STATE);
RHI_IMPL_POOL(PipelineStateDX9_t::ConstBuf,RESOURCE_CONST_BUFFER);


//------------------------------------------------------------------------------


PipelineStateDX9_t::ConstBuf::ConstBuf()
  : value(nullptr),
    inst(nullptr),
    reg(InvalidIndex),
    regCount(0)
{
}


//------------------------------------------------------------------------------

PipelineStateDX9_t::ConstBuf::~ConstBuf()
{
    if( value )
    {
        ::free( value );
        value = nullptr;
    }
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::Construct( ProgType ptype, unsigned reg_i, unsigned reg_count )
{
    DVASSERT(!value);

    progType = ptype;
    value    = (float*)(malloc( reg_count*4*sizeof(float) ));
    inst     = nullptr;
    reg      = reg_i;
    regCount = reg_count;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::Destroy()
{
    if( value )
    {
        ::free( value );

        value    = nullptr;
        inst     = nullptr;
        reg      = 0;
        regCount = 0;
    }
}


//------------------------------------------------------------------------------

unsigned
PipelineStateDX9_t::ConstBuf::ConstCount() const
{
    return regCount;
}


//------------------------------------------------------------------------------

const void*
PipelineStateDX9_t::ConstBuf::InstData() const
{
    if( !inst )
    {
        inst = _DefConstRingBuf.Alloc( 4*regCount );
        memcpy( inst, value, regCount*4*sizeof(float) );
    }

    return inst;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::InvalidateInst()
{
    inst = nullptr;
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::ConstBuf::SetConst( unsigned const_i, unsigned const_count, const float* data )
{
    bool    success = false;

    if( const_i + const_count <= regCount )
    {
        memcpy( value + const_i*4, data, const_count*4*sizeof(float) );
        inst    = nullptr;
        success = true;
    }
    
    return success;
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::ConstBuf::SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount )
{
    bool    success = false;

    if( const_i <= regCount  &&  const_sub_i < 4 )
    {
        memcpy( value + const_i*4 + const_sub_i, data, dataCount*sizeof(float) );
        inst    = nullptr;
        success = true;
    }
    
    return success;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::SetToRHI( const void* inst_data ) const
{
    if( progType == PROG_VERTEX )
        _D3D9_Device->SetVertexShaderConstantF( reg, (const float*)inst_data, regCount );
    else
        _D3D9_Device->SetPixelShaderConstantF( reg, (const float*)inst_data, regCount );    
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::VertexProgDX9::Construct( const void* bin, unsigned bin_sz, const VertexLayout& vdecl )
{
    bool                success     = false;
    LPD3DXBUFFER        shader      = NULL;
    LPD3DXBUFFER        err         = NULL;
    LPD3DXCONSTANTTABLE const_tab   = NULL;
    HRESULT             hr          = D3DXCompileShader
    (
        (const char*)bin, bin_sz,
        NULL, // no defines
        NULL, // no include-interfaces
        "vp_main",
        "vs_3_0",
        0,
        &shader,
        &err,
        &const_tab
    );

    if( SUCCEEDED(hr) )
    {
        void*   code = shader->GetBufferPointer();
        HRESULT hr   = _D3D9_Device->CreateVertexShader( (const DWORD*)code, &vs9 );

        if( SUCCEEDED(hr) )
        {
            for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
            {
                char        name[16];   sprintf( name, "VP_Buffer%u", i );
                D3DXHANDLE  c =         const_tab->GetConstantByName( NULL, name );
             
                cbufReg[i]   = InvalidIndex;
                cbufCount[i] = 0;

                if( c )
                {
                    D3DXCONSTANT_DESC   desc;
                    UINT                cnt = 1;

                    hr = const_tab->GetConstantDesc( c, &desc, &cnt );
                    
                    if( SUCCEEDED(hr) )
                    {
                        cbufReg[i]   = desc.RegisterIndex;
                        cbufCount[i] = desc.Elements;
                    }
                }
            }

            vdecl9 = VDeclDX9::Get( vdecl );
            stride = vdecl.Stride();

            DVASSERT(vdecl9);
            success = true;
        }
        else
        {
            Logger::Error( "FAILED to create vertex-shader:\n%s\n", D3D9ErrorText(hr) );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile vertex-shader:\n" );
        if( err )
        {
            Logger::Info( (const char*)(err->GetBufferPointer()) );
        }
        Logger::Error( "vertex-shader text:\n" );
        DumpShaderText( (const char*)bin, bin_sz );
    }

    return success;
}


//------------------------------------------------------------------------------

Handle 
PipelineStateDX9_t::VertexProgDX9::CreateConstBuffer( unsigned buf_i )
{
    Handle  handle = InvalidHandle;

    DVASSERT(buf_i < MAX_CONST_BUFFER_COUNT);
    
    if( buf_i < MAX_CONST_BUFFER_COUNT )
    {
        handle = ConstBufDX9Pool::Alloc();

        ConstBuf*   cb = ConstBufDX9Pool::Get( handle );

        cb->Construct( PROG_VERTEX, cbufReg[buf_i], cbufCount[buf_i] );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::VertexProgDX9::SetToRHI( uint32 layoutUID )
{
    HRESULT hr = _D3D9_Device->SetVertexShader( vs9 );

    if( SUCCEEDED(hr) )
    {
        IDirect3DVertexDeclaration9*    vd = vdecl9;

        if( layoutUID != VertexLayout::InvalidUID )
        {
            bool    do_add = true;

            for( std::vector<vdecl_t>::iterator i=altVdecl9.begin(),i_end=altVdecl9.end(); i!=i_end; ++i )
            {
                if( i->layoutUID == layoutUID )
                {
                    vd     = i->vdecl;
                    do_add = false;
                    break;
                }
            }

            if( do_add )
            {
                const VertexLayout* layout = VertexLayout::Get( layoutUID );
                vdecl_t             info;
                
                info.vdecl      = VDeclDX9::Get( *layout );
                info.layoutUID  = layoutUID;

                altVdecl9.push_back( info );
                vd = info.vdecl;
            }
        }

        hr = _D3D9_Device->SetVertexDeclaration( vd );

        if( FAILED(hr) )
            Logger::Error( "SetVertexDeclaration failed:\n%s\n", D3D9ErrorText(hr) );        
    }
    else
    {
        Logger::Error( "SetVertexShader failed:\n%s\n", D3D9ErrorText(hr) );
    }
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::FragmentProgDX9::Construct( const void* bin, unsigned bin_sz )
{
    bool                success     = false;
    LPD3DXBUFFER        shader      = NULL;
    LPD3DXBUFFER        err         = NULL;
    LPD3DXCONSTANTTABLE const_tab   = NULL;
    HRESULT             hr          = D3DXCompileShader
    (
        (const char*)bin, bin_sz,
        NULL, // no defines
        NULL, // no include-interfaces
        "fp_main",
        "ps_3_0",
        0,
        &shader,
        &err,
        &const_tab
    );

    if( SUCCEEDED(hr) )
    {
        void*   code = shader->GetBufferPointer();
        HRESULT hr   = _D3D9_Device->CreatePixelShader( (const DWORD*)code, &ps9 );

        if( SUCCEEDED(hr) )
        {
            for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
            {
                char        name[16];   sprintf( name, "FP_Buffer%u", i );
                D3DXHANDLE  c =         const_tab->GetConstantByName( NULL, name );
             
                cbufReg[i]   = InvalidIndex;
                cbufCount[i] = 0;

                if( c )
                {
                    D3DXCONSTANT_DESC   desc;
                    UINT                cnt = 1;

                    hr = const_tab->GetConstantDesc( c, &desc, &cnt );
                    
                    if( SUCCEEDED(hr) )
                    {
                        cbufReg[i]   = desc.RegisterIndex;
                        cbufCount[i] = desc.Elements;
                    }
                }
            }

            success = true;
        }
        else
        {
            Logger::Error( "FAILED to create pixel-shader:\n%s\n", D3D9ErrorText(hr) );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile pixel-shader:\n" );
        if( err )
        {
            Logger::Info( (const char*)(err->GetBufferPointer()) );
        }
        Logger::Error( "fragment-shader text:\n" );
        DumpShaderText( (const char*)bin, bin_sz );
    }

    return success;
}


//------------------------------------------------------------------------------

Handle 
PipelineStateDX9_t::FragmentProgDX9::CreateConstBuffer( unsigned buf_i )
{
    Handle  handle = InvalidHandle;

    DVASSERT(buf_i < MAX_CONST_BUFFER_COUNT);
    
    if( buf_i < MAX_CONST_BUFFER_COUNT )
    {
        handle = ConstBufDX9Pool::Alloc();

        ConstBuf*   cb = ConstBufDX9Pool::Get( handle );

        cb->Construct( PROG_FRAGMENT, cbufReg[buf_i], cbufCount[buf_i] );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::FragmentProgDX9::SetToRHI()
{
    HRESULT hr = _D3D9_Device->SetPixelShader( ps9 );

    if( FAILED(hr) )
        Logger::Error( "SetPixelShader failed:\n%s\n", D3D9ErrorText(hr) );
}


//==============================================================================

static Handle
dx9_PipelineState_Create(const PipelineState::Descriptor& desc)
{
    Handle                      handle      = PipelineStateDX9Pool::Alloc();
    PipelineStateDX9_t*         ps          = PipelineStateDX9Pool::Get( handle );
    bool                        vprog_valid = false;
    bool                        fprog_valid = false;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );

    vprog_valid = ps->vprog.Construct( (const char*)(&vprog_bin[0]), vprog_bin.size(), desc.vertexLayout ); 
    fprog_valid = ps->fprog.Construct( (const char*)(&fprog_bin[0]), fprog_bin.size() ); 
    
    if( vprog_valid  &&  fprog_valid )
    {
        ps->blendEnabled = desc.blending.rtBlend[0].blendEnabled;
    
        switch( desc.blending.rtBlend[0].colorSrc )
        {
            case BLENDOP_ZERO           : ps->blendSrc = D3DBLEND_ZERO; break;
            case BLENDOP_ONE            : ps->blendSrc = D3DBLEND_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendSrc = D3DBLEND_SRCALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendSrc = D3DBLEND_INVSRCALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendSrc = D3DBLEND_SRCCOLOR; break;
        }
        
        switch( desc.blending.rtBlend[0].colorDst )
        {
            case BLENDOP_ZERO           : ps->blendDst = D3DBLEND_ZERO; break;
            case BLENDOP_ONE            : ps->blendDst = D3DBLEND_ONE; break;
            case BLENDOP_SRC_ALPHA      : ps->blendDst = D3DBLEND_SRCALPHA; break;
            case BLENDOP_INV_SRC_ALPHA  : ps->blendDst = D3DBLEND_INVSRCALPHA; break;
            case BLENDOP_SRC_COLOR      : ps->blendDst = D3DBLEND_SRCCOLOR; break;
        }
    }
    else
    {
        PipelineStateDX9Pool::Free( handle );
        handle = InvalidHandle;
    }

    return handle;
}


static void
dx9_PipelineState_Delete( Handle ps )
{
}


//------------------------------------------------------------------------------

static Handle
dx9_PipelineState_CreateVertexConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );
    
    return ps9->vprog.CreateConstBuffer( buf_i );
}


//------------------------------------------------------------------------------

static Handle
dx9_PipelineState_CreateFragmentConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );
    
    return ps9->fprog.CreateConstBuffer( buf_i );
}



namespace PipelineStateDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_PipelineState_Create                     = &dx9_PipelineState_Create;
    dispatch->impl_PipelineState_Delete                     = &dx9_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer    = &dx9_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer  = &dx9_PipelineState_CreateFragmentConstBuffer;
}


//------------------------------------------------------------------------------

void
SetToRHI( Handle ps, uint32 layoutUID )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );

    ps9->vprog.SetToRHI( layoutUID );
    ps9->fprog.SetToRHI();

    if( ps9->blendEnabled )
    {
        _D3D9_Device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
        _D3D9_Device->SetRenderState( D3DRS_SRCBLEND, ps9->blendSrc );
        _D3D9_Device->SetRenderState( D3DRS_DESTBLEND, ps9->blendDst );
    }
    else
    {
        _D3D9_Device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
    }
}


//------------------------------------------------------------------------------

unsigned
VertexLayoutStride( Handle ps )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );

    return ps9->vprog.stride;
}


} // namespace PipelineStateDX9


//------------------------------------------------------------------------------

static bool
dx9_ConstBuffer_SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );

    return cb9->SetConst( const_i, const_count, data );
}


//------------------------------------------------------------------------------

static bool
dx9_ConstBuffer_SetConst1fv( Handle cb, unsigned const_i, unsigned const_sub_i, const float* data, uint32 dataCount )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );

    return cb9->SetConst( const_i, const_sub_i, data, dataCount );
}


//------------------------------------------------------------------------------

void
dx9_ConstBuffer_Delete( Handle cb )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );
    
    cb9->Destroy();    
    ConstBufDX9Pool::Free( cb );
}


//------------------------------------------------------------------------------

namespace ConstBufferDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_ConstBuffer_SetConst     = &dx9_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv  = &dx9_ConstBuffer_SetConst1fv;
    dispatch->impl_ConstBuffer_Delete       = &dx9_ConstBuffer_Delete;
}

void
InitializeRingBuffer( uint32 size )
{
    _DefConstRingBuf.Initialize( size );
}


const void*
InstData( Handle cb )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );
    
    return cb9->InstData();
}

void
SetToRHI( Handle cb, const void* instData )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );
    
    cb9->SetToRHI( instData );
}


void
InvalidateAllConstBufferInstances()
{
    for( ConstBufDX9Pool::Iterator b=ConstBufDX9Pool::Begin(),b_end=ConstBufDX9Pool::End(); b!=b_end; ++b )
    {
        b->InvalidateInst();
    }
}

}


//==============================================================================
} // namespace rhi

