//  externals:

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../rhi_ShaderCache.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    using DAVA::uint32;
    using DAVA::uint16;
    using DAVA::uint8;

    #include "_dx11.h"
    #include <D3DX11async.h>
    #include <D3D11Shader.h>
    #include <D3Dcompiler.h>

    #include <vector>


namespace rhi
{
//==============================================================================


ID3D11InputLayout*
_CreateInputLayout( const VertexLayout& layout, const void* code, unsigned code_sz, bool force_immediate )
{
    ID3D11InputLayout*          vdecl     = nullptr;
    D3D11_INPUT_ELEMENT_DESC    elem[32];
    uint32                      elemCount = 0;

    DVASSERT(layout.ElementCount() < countof(elem));
    for( unsigned i=0; i!=layout.ElementCount(); ++i )
    {
        if( layout.ElementSemantics(i) == VS_PAD )
            continue;

        elem[elemCount].AlignedByteOffset    = (UINT)(layout.ElementOffset( i ));
        elem[elemCount].SemanticIndex        = layout.ElementSemanticsIndex( i );
        elem[elemCount].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        elem[elemCount].InstanceDataStepRate = 0;

        switch( layout.ElementSemantics(i) )
        {
            case VS_POSITION : 
            {
                elem[elemCount].SemanticName = "POSITION"; 
                elem[elemCount].InputSlot    = VATTR_POSITION;
            }   break;

            case VS_NORMAL : 
            {
                elem[elemCount].SemanticName = "NORMAL"; 
                elem[elemCount].InputSlot    = VATTR_NORMAL;
            }   break;

            case VS_COLOR : 
            {
                elem[elemCount].SemanticName = "COLOR";
                switch( layout.ElementSemanticsIndex(i) )
                {
                    case 0  : elem[elemCount].InputSlot = VATTR_COLOR_0; break;
                    case 1  : elem[elemCount].InputSlot = VATTR_COLOR_1; break;
                }
            }   break;

            case VS_TEXCOORD : 
            {
                elem[elemCount].SemanticName = "TEXCOORDS";
                switch( layout.ElementSemanticsIndex(i) )
                {
                    case 0  : elem[elemCount].InputSlot = VATTR_TEXCOORD_0; break;
                    case 1  : elem[elemCount].InputSlot = VATTR_TEXCOORD_1; break;
                    case 2  : elem[elemCount].InputSlot = VATTR_TEXCOORD_2; break;
                    case 3  : elem[elemCount].InputSlot = VATTR_TEXCOORD_3; break;
                    case 4  : elem[elemCount].InputSlot = VATTR_TEXCOORD_4; break;
                    case 5  : elem[elemCount].InputSlot = VATTR_TEXCOORD_5; break;
                    case 6  : elem[elemCount].InputSlot = VATTR_TEXCOORD_6; break;
                    case 7  : elem[elemCount].InputSlot = VATTR_TEXCOORD_7; break;
                }
            }   break;

            case VS_TANGENT :
            {
                elem[elemCount].SemanticName = "TANGENT";
                elem[elemCount].InputSlot    = VATTR_TANGENT;
            }   break;

            case VS_BINORMAL: 
            {
                elem[elemCount].SemanticName = "BINORMAL";
                elem[elemCount].InputSlot    = VATTR_BINORMAL;
            }   break;

            case VS_BLENDWEIGHT : 
            {
                elem[elemCount].SemanticName = "BLENDWEIGHT";
                elem[elemCount].InputSlot    = VATTR_BLENDWEIGHT;
            }   break;
                
            case VS_BLENDINDEX :
            {
                elem[elemCount].SemanticName = "BLENDINDEX";
                elem[elemCount].InputSlot    = VATTR_BLENDINDEX;
            }   break;
        }

        switch( layout.ElementDataType(i) )
        {
            case VDT_FLOAT :
            {
                switch( layout.ElementDataCount(i) )
                {
                    case 4 : elem[elemCount].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                    case 3 : elem[elemCount].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
                    case 2 : elem[elemCount].Format = DXGI_FORMAT_R32G32_FLOAT; break;
                    case 1 : elem[elemCount].Format = DXGI_FORMAT_R32_FLOAT; break;
                }
            }   break;
        }

        if( layout.ElementSemantics(i) == VS_COLOR )
        {
            elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM ; break;
        }

        ++elemCount;
    }

    HRESULT hr = _D3D11_Device->CreateInputLayout( elem, elemCount, code, code_sz, &vdecl );

    if( FAILED(hr) )
    {
    }

    return vdecl;
}



//==============================================================================

class
ConstBufDX11
{
public:
                ConstBufDX11();
                ~ConstBufDX11();
    
    void        Construct( ProgType type, unsigned buf_i, unsigned reg_count );
    void        Destroy();

    unsigned    ConstCount() const;
    const void* InstData() const;
    void        InvalidateInst();

    bool        SetConst( unsigned const_i, unsigned count, const float* data );
    bool        SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount );
    void        SetToRHI() const;


private:

    void        _EnsureMapped();

    ProgType        progType;
    ID3D11Buffer*   buf;
    mutable float*  value;
    unsigned        buf_i;
    unsigned        regCount;
};

//------------------------------------------------------------------------------


ConstBufDX11::ConstBufDX11()
  : buf(nullptr),
    value(nullptr),
    buf_i(InvalidIndex),
    regCount(0)
{
}


//------------------------------------------------------------------------------

ConstBufDX11::~ConstBufDX11()
{
}


//------------------------------------------------------------------------------

void
ConstBufDX11::Construct( ProgType ptype, unsigned bufIndex, unsigned regCnt )
{
    DVASSERT(!value);
    DVASSERT(bufIndex != InvalidIndex);
    DVASSERT(regCnt);

    D3D11_BUFFER_DESC   desc = {0};
//-    ID3D11Buffer*       buf  = nullptr;
    DX11Command         cmd  = { DX11Command::CREATE_BUFFER, { uint64_t(&desc), NULL, uint64_t(&buf) } };
        
    desc.ByteWidth        = regCnt*4*sizeof(float);
    desc.Usage            = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags        = 0;
        
    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        progType = ptype;
        value    = nullptr;
        buf_i    = bufIndex;
        regCount = regCnt;
    }
    else
    {
        Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D11ErrorText(cmd.retval) );
    }
}


//------------------------------------------------------------------------------

void
ConstBufDX11::Destroy()
{
    if( buf )
    {
        DX11Command cmd[] = 
        { 
            { DX11Command::UNMAP_RESOURCE, { uint64_t(static_cast<IUnknown*>(buf)), 0 } } ,
            { DX11Command::RELEASE, { uint64_t(static_cast<IUnknown*>(buf)) } } 
        };

        if( !value )
            cmd[0].func = DX11Command::NOP;

        ExecDX11( cmd, countof(cmd) );
        
        buf      = nullptr;
        value    = nullptr;
        buf_i    = InvalidIndex;
        regCount = 0;
    }
}


//------------------------------------------------------------------------------

unsigned
ConstBufDX11::ConstCount() const
{
    return regCount;
}


//------------------------------------------------------------------------------

void 
ConstBufDX11::_EnsureMapped()
{
    if( !value )
    {
        D3D11_MAPPED_SUBRESOURCE    rc  = {0};
        DX11Command                 cmd = { DX11Command::MAP_RESOURCE, { uint64_t(buf), 0, D3D11_MAP_WRITE_DISCARD, 0, uint64_t(&rc) } };
        
        ExecDX11( &cmd, 1 );
        value = (float*)rc.pData;
    }
}


//------------------------------------------------------------------------------

bool
ConstBufDX11::SetConst( unsigned const_i, unsigned const_count, const float* data )
{
    bool    success = false;

    if( const_i + const_count <= regCount )
    {
        _EnsureMapped();
        memcpy( value + const_i*4, data, const_count*4*sizeof(float) );
        success = true;
    }
    
    return success;
}


//------------------------------------------------------------------------------

bool
ConstBufDX11::SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount )
{
    bool    success = false;

    if( const_i <= regCount  &&  const_sub_i < 4 )
    {
        _EnsureMapped();
        memcpy( value + const_i*4 + const_sub_i, data, dataCount*sizeof(float) );
        success = true;
    }
    
    return success;
}


//------------------------------------------------------------------------------

void
ConstBufDX11::SetToRHI() const
{
    if( value )
    {
        _D3D11_ImmediateContext->Unmap( buf, 0 );
        value = nullptr;
    }
    
    ID3D11Buffer*   cb[1] = { buf };

    if( progType == PROG_VERTEX )
        _D3D11_ImmediateContext->VSSetConstantBuffers( buf_i, 1, &buf );
    else
        _D3D11_ImmediateContext->PSSetConstantBuffers( buf_i, 1, &buf );
}

//==============================================================================

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


//==============================================================================

class
PipelineStateDX11_t
{
public:

    Handle  CreateConstBuffer( ProgType type, unsigned buf_i );


    ID3D11VertexShader* _vs11;
    unsigned            _vp_buf_count;
    unsigned            _vp_buf_reg_count[16];

    ID3D11PixelShader*  _ps11;
    unsigned            _fp_buf_count;
    unsigned            _fp_buf_reg_count[16];

    VertexLayout        _layout;
    ID3D11InputLayout*  _layout11;
};

typedef ResourcePool<PipelineStateDX11_t,RESOURCE_PIPELINE_STATE>   PipelineStateDX11Pool;
typedef ResourcePool<ConstBufDX11,RESOURCE_CONST_BUFFER>            ConstBufDX11Pool;

RHI_IMPL_POOL(PipelineStateDX11_t,RESOURCE_PIPELINE_STATE);
RHI_IMPL_POOL_SIZE(ConstBufDX11,RESOURCE_CONST_BUFFER,8*1024);


//------------------------------------------------------------------------------

Handle
PipelineStateDX11_t::CreateConstBuffer( ProgType type, unsigned buf_i )
{
    Handle          handle = ConstBufDX11Pool::Alloc();
    ConstBufDX11*   cb     = ConstBufDX11Pool::Get( handle );

    cb->Construct( type, buf_i, (type==PROG_VERTEX) ? _vp_buf_reg_count[buf_i] : _fp_buf_reg_count[buf_i] );

    return handle;
}


//==============================================================================

static Handle
dx11_PipelineState_Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = PipelineStateDX11Pool::Alloc();
    PipelineStateDX11_t*        ps          = PipelineStateDX11Pool::Get( handle );
    HRESULT                     hr;
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;
    ID3D10Blob*                 vp_code     = nullptr;
    ID3D10Blob*                 vp_err      = nullptr;
    ID3D10Blob*                 fp_code     = nullptr;
    ID3D10Blob*                 fp_err      = nullptr;

//Logger::Info("create PS");
//Logger::Info("  vprog= %s",desc.vprogUid.c_str());
//Logger::Info("  fprog= %s",desc.vprogUid.c_str());
//desc.vertexLayout.Dump();
    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );


    // create vertex-shader

    hr = D3DX11CompileFromMemory
    (
        (const char*)(&vprog_bin[0]), vprog_bin.size(),
        "vprog",
        NULL, // no macros
        NULL, // no includes
        "vp_main",
        "vs_4_0_level_9_1",
        D3D10_SHADER_OPTIMIZATION_LEVEL2,
        0, // no effect compile flags
        NULL, // synchronous execution
        &vp_code,
        &vp_err,
        NULL // no async-result needed
    );
    
    if( SUCCEEDED(hr) )
    {
        hr = _D3D11_Device->CreateVertexShader( vp_code->GetBufferPointer(), vp_code->GetBufferSize(), NULL, &(ps->_vs11) );
        
        if( SUCCEEDED(hr) )
        {
            ID3D11ShaderReflection* reflection = NULL; 
            D3D11_SHADER_DESC       desc       = {0};
            
            hr = D3DReflect( vp_code->GetBufferPointer(), vp_code->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection );
            
            if( SUCCEEDED(hr) )
            {
                hr = reflection->GetDesc( &desc );

                if( SUCCEEDED(hr) )
                {
                    ps->_vp_buf_count = desc.ConstantBuffers;
                    
                    for( unsigned b=0; b!=desc.ConstantBuffers; ++b )
                    {
                        ID3D11ShaderReflectionConstantBuffer*   cb = reflection->GetConstantBufferByIndex( b );
                        
                        if( cb )
                        {
                            D3D11_SHADER_BUFFER_DESC    cb_desc;

                            hr = cb->GetDesc( &cb_desc );
                            if( SUCCEEDED(hr) )
                            {
                                ps->_vp_buf_reg_count[b] = cb_desc.Size/(4*sizeof(float));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            Logger::Error( "FAILED to create vertex-shader:\n%s\n", D3D11ErrorText(hr) );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile vertex-shader:" );
        if( vp_err )
        {
            Logger::Info( (const char*)(vp_err->GetBufferPointer()) );
        }
        Logger::Error( "shader-uid : %s", desc.vprogUid.c_str() );
        Logger::Error( "vertex-shader text:\n" );
        DumpShaderText( (const char*)(&vprog_bin[0]), vprog_bin.size() );
    }


    // create fragment-shader

    hr = D3DX11CompileFromMemory
    (
        (const char*)(&fprog_bin[0]), fprog_bin.size(),
        "fprog",
        NULL, // no macros
        NULL, // no includes
        "fp_main",
        "ps_4_0_level_9_1",
        D3D10_SHADER_OPTIMIZATION_LEVEL2,
        0, // no effect compile flags
        NULL, // synchronous execution
        &fp_code,
        &fp_err,
        NULL // no async-result needed
    );
    
    if( SUCCEEDED(hr) )
    {
        hr = _D3D11_Device->CreatePixelShader( fp_code->GetBufferPointer(), fp_code->GetBufferSize(), NULL, &(ps->_ps11) );
        
        if( SUCCEEDED(hr) )
        {
            ID3D11ShaderReflection* reflection = NULL; 
            D3D11_SHADER_DESC       desc       = {0};
            
            hr = D3DReflect( fp_code->GetBufferPointer(), fp_code->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection );
            
            if( SUCCEEDED(hr) )
            {
                hr = reflection->GetDesc( &desc );

                if( SUCCEEDED(hr) )
                {
                    ps->_fp_buf_count = desc.ConstantBuffers;

                    for( unsigned b=0; b!=desc.ConstantBuffers; ++b )
                    {
                        ID3D11ShaderReflectionConstantBuffer*   cb = reflection->GetConstantBufferByIndex( b );
                        
                        if( cb )
                        {
                            D3D11_SHADER_BUFFER_DESC    cb_desc;

                            hr = cb->GetDesc( &cb_desc );
                            if( SUCCEEDED(hr) )
                            {
                                ps->_fp_buf_reg_count[b] = cb_desc.Size/(4*sizeof(float));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            Logger::Error( "FAILED to create pixel-shader:\n%s\n", D3D11ErrorText(hr) );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile pixel-shader:" );
        if( fp_err )
        {
            Logger::Info( (const char*)(fp_err->GetBufferPointer()) );
        }
        Logger::Error( "shader-uid : %s", desc.fprogUid.c_str() );
        Logger::Error( "vertex-shader text:\n" );
        DumpShaderText( (const char*)(&fprog_bin[0]), fprog_bin.size() );
    }


    // create input-layout

    ps->_layout11 = _CreateInputLayout( desc.vertexLayout, vp_code->GetBufferPointer(), vp_code->GetBufferSize(), true );
    ps->_layout   = desc.vertexLayout;

    return handle;
}


//------------------------------------------------------------------------------

static void
dx11_PipelineState_Delete( Handle ps )
{
    PipelineStateDX11Pool::Free( ps ); 
}


//------------------------------------------------------------------------------

static Handle
dx11_PipelineState_CreateVertexConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get( ps );
    
    return ps11->CreateConstBuffer( PROG_VERTEX, buf_i );
}


//------------------------------------------------------------------------------

static Handle
dx11_PipelineState_CreateFragmentConstBuffer( Handle ps, unsigned buf_i )
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get( ps );
    
    return ps11->CreateConstBuffer( PROG_FRAGMENT, buf_i );
}



//------------------------------------------------------------------------------

static bool
dx11_ConstBuffer_SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );

    return cb11->SetConst( const_i, const_count, data );
}


//------------------------------------------------------------------------------

static bool
dx11_ConstBuffer_SetConst1fv( Handle cb, unsigned const_i, unsigned const_sub_i, const float* data, uint32 dataCount )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );

    return cb11->SetConst( const_i, const_sub_i, data, dataCount );
}


//------------------------------------------------------------------------------

void
dx11_ConstBuffer_Delete( Handle cb )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );
    
    cb11->Destroy();
    ConstBufDX11Pool::Free( cb );
}


namespace PipelineStateDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_PipelineState_Create                     = &dx11_PipelineState_Create;
    dispatch->impl_PipelineState_Delete                     = &dx11_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer    = &dx11_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer  = &dx11_PipelineState_CreateFragmentConstBuffer;
}


//------------------------------------------------------------------------------

void
SetToRHI( Handle ps, uint32 layoutUID )
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get( ps );
    
    _D3D11_ImmediateContext->IASetInputLayout( ps11->_layout11 );
    _D3D11_ImmediateContext->VSSetShader( ps11->_vs11, NULL, 0 );
    _D3D11_ImmediateContext->PSSetShader( ps11->_ps11, NULL, 0 );
}

unsigned
VertexLayoutStride( Handle ps )
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get( ps );
    
    return ps11->_layout.Stride();
}

} // namespace PipelineStateDX9



namespace ConstBufferDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_ConstBuffer_SetConst     = &dx11_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv  = &dx11_ConstBuffer_SetConst1fv;
    dispatch->impl_ConstBuffer_Delete       = &dx11_ConstBuffer_Delete;
}

void
SetToRHI( Handle cb )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );
    
    cb11->SetToRHI();
}

}



} // namespace rhi