//  externals:

    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
    #include "../RHI/rhi_RingBuffer.h"
    #include "../RHI/rhi_ShaderCache.h"
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



//------------------------------------------------------------------------------

IDirect3DVertexDeclaration9*
VDeclDX9::Get( const VertexLayout& layout )
{
    IDirect3DVertexDeclaration9*    vdecl = 0;

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
        IDirect3DVertexDeclaration9*    vd9     = 0;
        D3DVERTEXELEMENT9               elem[32];
        HRESULT                         hr;

        DVASSERT(layout.ElementCount() < countof(elem));
        for( unsigned i=0; i!=layout.ElementCount(); ++i )
        {
            elem[i].Stream      = 0;
            elem[i].Offset      = (WORD)(layout.ElementOffset( i ));
            elem[i].Method      = D3DDECLMETHOD_DEFAULT;
            elem[i].UsageIndex  = 0;

            switch( layout.ElementSemantics(i) )
            {
                case VS_POSITION    : elem[i].Usage = D3DDECLUSAGE_POSITION; break;
                case VS_NORMAL      : elem[i].Usage = D3DDECLUSAGE_NORMAL; break;
                case VS_COLOR       : elem[i].Usage = D3DDECLUSAGE_COLOR; break;
                case VS_TEXCOORD    : elem[i].Usage = D3DDECLUSAGE_TEXCOORD; break;
                case VS_TANGENT     : elem[i].Usage = D3DDECLUSAGE_TANGENT; break;
                case VS_BINORMAL    : elem[i].Usage = D3DDECLUSAGE_BINORMAL; break;
                case VS_BLENDWEIGHT : elem[i].Usage = D3DDECLUSAGE_BLENDWEIGHT; break;
                case VS_BLENDINDEX  : elem[i].Usage = D3DDECLUSAGE_BLENDINDICES; break;
            }

            switch( layout.ElementDataType(i) )
            {
                case VDT_FLOAT :
                {
                    switch( layout.ElementDataCount(i) )
                    {
                        case 4 : elem[i].Type = D3DDECLTYPE_FLOAT4; break;
                        case 3 : elem[i].Type = D3DDECLTYPE_FLOAT3; break;
                        case 2 : elem[i].Type = D3DDECLTYPE_FLOAT2; break;
                    }
                }   break;
            }
        }
        elem[layout.ElementCount()].Stream      = 0xFF;
        elem[layout.ElementCount()].Offset      = 0;
        elem[layout.ElementCount()].Type        = D3DDECLTYPE_UNUSED;
        elem[layout.ElementCount()].Method      = 0;
        elem[layout.ElementCount()].Usage       = 0;
        elem[layout.ElementCount()].UsageIndex  = 0;
        

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
    
        void        construct( ProgType type, unsigned reg_i, unsigned reg_count );

        unsigned    const_count() const;
        const void* inst_data() const;

        bool        set_const( unsigned const_i, unsigned count, const float* data );
        void        set_to_rhi( const void* inst_data ) const;


    private:

        float*          _value;
        mutable float*  _value_inst;
        unsigned        _reg_i;
        unsigned        _reg_count;
        ProgType        _type;
        mutable uint32  _inst_valid:1;
    };

    struct
    VertexProgDX9
    {
    public:

                                        VertexProgDX9()
                                          : stride(0),
                                            code_size(0),
                                            code(0),
                                            vs9(0),
                                            vdecl9(0)
                                        {
                                        }

        bool                            construct( const void* code, unsigned code_sz, const VertexLayout& vdecl );
        Handle                          create_const_buffer( unsigned buf_i );
        void                            set_to_rhi();
        
        unsigned                        stride;
        unsigned                        code_size;
        void*                           code;
        IDirect3DVertexShader9*         vs9;
        IDirect3DVertexDeclaration9*    vdecl9;    // ref-only
        unsigned                        cbuf_reg[MAX_CONST_BUFFER_COUNT];
        unsigned                        cbuf_count[MAX_CONST_BUFFER_COUNT];
    };

    struct
    FragmentProgDX9
    {
    public:

                                FragmentProgDX9()
                                  : code_size(0),
                                    code(0),
                                    ps9(0)
                                {
                                }

        bool                    construct( const void* code, unsigned code_sz );
        Handle                  create_const_buffer( unsigned buf_i );
        void                    set_to_rhi();

        unsigned                code_size;
        void*                   code;
        IDirect3DPixelShader9*  ps9;
        unsigned                cbuf_reg[MAX_CONST_BUFFER_COUNT];
        unsigned                cbuf_count[MAX_CONST_BUFFER_COUNT];
    };

    VertexProgDX9       vprog;
    FragmentProgDX9     fprog;
};

typedef Pool<PipelineStateDX9_t,RESOURCE_PIPELINE_STATE>            PipelineStateDX9Pool;
typedef Pool<PipelineStateDX9_t::ConstBuf,RESOURCE_CONST_BUFFER>    ConstBufDX9Pool;

RHI_IMPL_POOL(PipelineStateDX9_t,RESOURCE_PIPELINE_STATE);
RHI_IMPL_POOL(PipelineStateDX9_t::ConstBuf,RESOURCE_CONST_BUFFER);


//------------------------------------------------------------------------------


PipelineStateDX9_t::ConstBuf::ConstBuf()
  : _value(0),
    _value_inst(0),
    _reg_i(InvalidIndex),
    _reg_count(0),
    _inst_valid(false)
{
}


//------------------------------------------------------------------------------

PipelineStateDX9_t::ConstBuf::~ConstBuf()
{
    if( _value )
    {
        ::free( _value );
        _value = 0;
    }
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::construct( ProgType type, unsigned reg_i, unsigned reg_count )
{
    DVASSERT(!_value);

    _type       = type;
    _value      = (float*)(malloc( reg_count*4*sizeof(float) ));
    _reg_i      = reg_i;
    _reg_count  = reg_count; 
    _inst_valid = false;
}


//------------------------------------------------------------------------------

unsigned
PipelineStateDX9_t::ConstBuf::const_count() const
{
    return _reg_count;
}


//------------------------------------------------------------------------------

const void*
PipelineStateDX9_t::ConstBuf::inst_data() const
{
    if( !_inst_valid )
    {
        _value_inst = _DefConstRingBuf.Alloc( 4*_reg_count );
        memcpy( _value_inst, _value, _reg_count*4*sizeof(float) );
        _inst_valid = true;
    }

    return _value_inst;
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::ConstBuf::set_const( unsigned const_i, unsigned const_count, const float* data )
{
    bool    success = false;

    if( const_i + const_count <= _reg_count )
    {
        memcpy( _value + const_i*4, data, const_count*4*sizeof(float) );
        _inst_valid = false;
    }
    
    return success;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::ConstBuf::set_to_rhi( const void* inst_data ) const
{
    if( _type == PROG_VERTEX )
        _D3D9_Device->SetVertexShaderConstantF( _reg_i, (const float*)inst_data, _reg_count );
    else
        _D3D9_Device->SetPixelShaderConstantF( _reg_i, (const float*)inst_data, _reg_count );
}


//------------------------------------------------------------------------------

bool
PipelineStateDX9_t::VertexProgDX9::construct( const void* bin, unsigned bin_sz, const VertexLayout& vdecl )
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
        "vs_2_0",
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
             
                cbuf_reg[i]   = InvalidIndex;
                cbuf_count[i] = 0;

                if( c )
                {
                    D3DXCONSTANT_DESC   desc;
                    UINT                cnt = 1;

                    hr = const_tab->GetConstantDesc( c, &desc, &cnt );
                    
                    if( SUCCEEDED(hr) )
                    {
                        cbuf_reg[i]   = desc.RegisterIndex;
                        cbuf_count[i] = desc.Elements;
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
    }

    return success;
}


//------------------------------------------------------------------------------

Handle 
PipelineStateDX9_t::VertexProgDX9::create_const_buffer( unsigned buf_i )
{
    Handle  handle = InvalidHandle;

    DVASSERT(buf_i < MAX_CONST_BUFFER_COUNT);
    
    if( buf_i < MAX_CONST_BUFFER_COUNT )
    {
        handle = ConstBufDX9Pool::Alloc();

        ConstBuf*   cb = ConstBufDX9Pool::Get( handle );

        cb->construct( PROG_VERTEX, cbuf_reg[buf_i], cbuf_count[buf_i] );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::VertexProgDX9::set_to_rhi()
{
    HRESULT hr = _D3D9_Device->SetVertexShader( vs9 );

    if( SUCCEEDED(hr) )
    {
        hr = _D3D9_Device->SetVertexDeclaration( vdecl9 );

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
PipelineStateDX9_t::FragmentProgDX9::construct( const void* bin, unsigned bin_sz )
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
        "ps_2_0",
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
             
                cbuf_reg[i]   = InvalidIndex;
                cbuf_count[i] = 0;

                if( c )
                {
                    D3DXCONSTANT_DESC   desc;
                    UINT                cnt = 1;

                    hr = const_tab->GetConstantDesc( c, &desc, &cnt );
                    
                    if( SUCCEEDED(hr) )
                    {
                        cbuf_reg[i]   = desc.RegisterIndex;
                        cbuf_count[i] = desc.Elements;
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
    }

    return success;
}


//------------------------------------------------------------------------------

Handle 
PipelineStateDX9_t::FragmentProgDX9::create_const_buffer( unsigned buf_i )
{
    Handle  handle = InvalidHandle;

    DVASSERT(buf_i < MAX_CONST_BUFFER_COUNT);
    
    if( buf_i < MAX_CONST_BUFFER_COUNT )
    {
        handle = ConstBufDX9Pool::Alloc();

        ConstBuf*   cb = ConstBufDX9Pool::Get( handle );

        cb->construct( PROG_FRAGMENT, cbuf_reg[buf_i], cbuf_count[buf_i] );
    }

    return handle;
}


//------------------------------------------------------------------------------

void
PipelineStateDX9_t::FragmentProgDX9::set_to_rhi()
{
    HRESULT hr = _D3D9_Device->SetPixelShader( ps9 );

    if( FAILED(hr) )
        Logger::Error( "SetPixelShader failed:\n%s\n", D3D9ErrorText(hr) );
}


//==============================================================================

namespace PipelineState
{

Handle
Create(const PipelineState::Descriptor& desc)
{
    Handle                      handle      = PipelineStateDX9Pool::Alloc();;
    PipelineStateDX9_t*         ps          = PipelineStateDX9Pool::Get( handle );
    bool                        vprog_valid = false;
    bool                        fprog_valid = false;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );



    vprog_valid = ps->vprog.construct( (const char*)(&vprog_bin[0]), vprog_bin.size(), desc.vertexLayout ); 
    fprog_valid = ps->fprog.construct( (const char*)(&fprog_bin[0]), fprog_bin.size() ); 


    return handle;
}


//------------------------------------------------------------------------------

Handle
CreateVProgConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );
    
    return ps9->vprog.create_const_buffer( buf_i );
}


//------------------------------------------------------------------------------

Handle
CreateFProgConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );
    
    return ps9->fprog.create_const_buffer( buf_i );
}


} // PipelineState



namespace PipelineStateDX9
{

//------------------------------------------------------------------------------

void
SetToRHI( Handle ps )
{
    PipelineStateDX9_t* ps9 = PipelineStateDX9Pool::Get( ps );

    ps9->vprog.set_to_rhi();
    ps9->fprog.set_to_rhi();
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

namespace ConstBuffer
{

bool
SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );

    return cb9->set_const( const_i, const_count, data );
}

}


//------------------------------------------------------------------------------

namespace ConstBufferDX9
{

void
InitializeRingBuffer( uint32 size )
{
    _DefConstRingBuf.Initialize( size );
}


const void*
InstData( Handle cb )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );
    
    return cb9->inst_data();
}

void
SetToRHI( Handle cb, const void* instData )
{
    PipelineStateDX9_t::ConstBuf* cb9 = ConstBufDX9Pool::Get( cb );
    
    cb9->set_to_rhi( instData );
}

}


//==============================================================================
} // namespace rhi

