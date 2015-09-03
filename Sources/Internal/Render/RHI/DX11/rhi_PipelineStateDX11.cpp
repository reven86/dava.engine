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
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_RingBuffer.h"
    #include "../rhi_ShaderCache.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    using DAVA::uint32;
    using DAVA::uint16;
    using DAVA::uint8;

    #include "_dx11.h"
    #include <D3D11Shader.h>
    #include <D3Dcompiler.h>

    #include <vector>


namespace rhi
{
//==============================================================================


ID3D11InputLayout*
_CreateInputLayout( const VertexLayout& layout, const void* code, unsigned code_sz )
{
    ID3D11InputLayout*          vdecl     = nullptr;
    D3D11_INPUT_ELEMENT_DESC    elem[32];
    uint32                      elemCount = 0;

//Logger::Info("create-dx11-layout");
    DVASSERT(layout.ElementCount() < countof(elem));
    for( unsigned i=0; i!=layout.ElementCount(); ++i )
    {
        if( layout.ElementSemantics(i) == VS_PAD )
            continue;

        elem[elemCount].AlignedByteOffset    = (UINT)(layout.ElementOffset( i ));
        elem[elemCount].SemanticIndex        = layout.ElementSemanticsIndex( i );
        elem[elemCount].InputSlot            = 0;
        elem[elemCount].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        elem[elemCount].InstanceDataStepRate = 0;

        switch( layout.ElementSemantics(i) )
        {
            case VS_POSITION : 
            {
                elem[elemCount].SemanticName = "POSITION"; 
            }   break;

            case VS_NORMAL : 
            {
                elem[elemCount].SemanticName = "NORMAL"; 
            }   break;

            case VS_COLOR : 
            {
                elem[elemCount].SemanticName = "COLOR";
            }   break;

            case VS_TEXCOORD : 
            {
                elem[elemCount].SemanticName = "TEXCOORD";
            }   break;

            case VS_TANGENT :
            {
                elem[elemCount].SemanticName = "TANGENT";
            }   break;

            case VS_BINORMAL: 
            {
                elem[elemCount].SemanticName = "BINORMAL";
            }   break;

            case VS_BLENDWEIGHT : 
            {
                elem[elemCount].SemanticName = "BLENDWEIGHT";
            }   break;
                
            case VS_BLENDINDEX :
            {
                elem[elemCount].SemanticName = "BLENDINDICES";                
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
            elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }

//Logger::Info( "elem[%u] %s", elemCount, elem[elemCount].SemanticName );
        ++elemCount;
    }

    HRESULT hr = _D3D11_Device->CreateInputLayout( elem, elemCount, code, code_sz, &vdecl );

    if( FAILED(hr) )
    {
    }

    return vdecl;
}


//------------------------------------------------------------------------------

ID3D11InputLayout*
_CreateCompatibleInputLayout( const VertexLayout& vbLayout, const VertexLayout& vprogLayout, const void* code, unsigned code_sz )
{
    ID3D11InputLayout*          vdecl     = nullptr;
    D3D11_INPUT_ELEMENT_DESC    elem[32];
    uint32                      elemCount = 0;

//Logger::Info("create-compatible-dx11-layout");
    DVASSERT(vbLayout.ElementCount() < countof(elem));
    for( unsigned i=0; i!=vprogLayout.ElementCount(); ++i )
    {
        DVASSERT( vprogLayout.ElementSemantics(i) != VS_PAD );
        
        unsigned    vb_elem_i = InvalidIndex;

        for( unsigned k=0; k!=vbLayout.ElementCount(); ++k )
        {
            if(     vbLayout.ElementSemantics(k) == vprogLayout.ElementSemantics(i) 
                &&  vbLayout.ElementSemanticsIndex(k) == vprogLayout.ElementSemanticsIndex(i)
              )
            {
                vb_elem_i = k;
                break;
            }
        }

        if( vb_elem_i != InvalidIndex )
        {
            elem[elemCount].AlignedByteOffset    = (UINT)(vbLayout.ElementOffset( vb_elem_i ));
            elem[elemCount].SemanticIndex        = vprogLayout.ElementSemanticsIndex( i );
            elem[elemCount].InputSlot            = 0;
            elem[elemCount].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
            elem[elemCount].InstanceDataStepRate = 0;

            switch( vbLayout.ElementSemantics(vb_elem_i) )
            {
                case VS_POSITION : 
                {
                    elem[elemCount].SemanticName = "POSITION"; 
                }   break;

                case VS_NORMAL : 
                {
                    elem[elemCount].SemanticName = "NORMAL"; 
                }   break;

                case VS_COLOR : 
                {
                    elem[elemCount].SemanticName = "COLOR";
                }   break;

                case VS_TEXCOORD : 
                {
                    elem[elemCount].SemanticName = "TEXCOORD";
                }   break;

                case VS_TANGENT :
                {
                    elem[elemCount].SemanticName = "TANGENT";
                }   break;

                case VS_BINORMAL: 
                {
                    elem[elemCount].SemanticName = "BINORMAL";
                }   break;

                case VS_BLENDWEIGHT : 
                {
                    elem[elemCount].SemanticName = "BLENDWEIGHT";
                }   break;
                
                case VS_BLENDINDEX :
                {
                    elem[elemCount].SemanticName = "BLENDINDICES";                
                }   break;
            }

            switch( vbLayout.ElementDataType(vb_elem_i) )
            {
                case VDT_FLOAT :
                {
                    switch( vbLayout.ElementDataCount(vb_elem_i) )
                    {
                        case 4 : elem[elemCount].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                        case 3 : elem[elemCount].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
                        case 2 : elem[elemCount].Format = DXGI_FORMAT_R32G32_FLOAT; break;
                        case 1 : elem[elemCount].Format = DXGI_FORMAT_R32_FLOAT; break;
                    }
                }   break;
            }

            if( vbLayout.ElementSemantics(vb_elem_i) == VS_COLOR )
            {
                elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }
        else
        {
DVASSERT(!"kaboom!");
//            Logger::Error();
        }

//Logger::Info( "elem[%u] %s/%u", elemCount, elem[elemCount].SemanticName, elem[elemCount].SemanticIndex );
        ++elemCount;
    }

    if( vprogLayout.Stride() < vbLayout.Stride() )
    {
        const unsigned  padCnt = vbLayout.Stride() - vprogLayout.Stride();

        DVASSERT(padCnt%4 == 0);
        for( unsigned p=0; p!=padCnt/4; ++p )
        {
            elem[elemCount].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;//vprogLayout.Stride() + p;
            elem[elemCount].SemanticIndex        = p;
            elem[elemCount].SemanticName         = "PAD";
            elem[elemCount].InputSlot            = 0;
            elem[elemCount].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
            elem[elemCount].InstanceDataStepRate = 0;
            elem[elemCount].Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
//Logger::Info( "elem[%u] %s/%u", elemCount, elem[elemCount].SemanticName, elem[elemCount].SemanticIndex );
            
            ++elemCount;
        }
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

    struct Desc {};

                ConstBufDX11();
                ~ConstBufDX11();
    
    void        Construct( ProgType type, unsigned buf_i, unsigned reg_count );
    void        Destroy();

    unsigned    ConstCount() const;
    const void* InstData() const;
    void        InvalidateInst();

    bool        SetConst( unsigned const_i, unsigned count, const float* data );
    bool        SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount );
    void        SetToRHI( const void* inst_data, ID3D11DeviceContext* context ) const;


private:

    void        _EnsureMapped();

    ProgType        progType;
    ID3D11Buffer*   buf;
    mutable float*  value;
    mutable float*  inst;
    unsigned        buf_i;
    unsigned        regCount;
};

static RingBuffer   _DefConstRingBuf;


//------------------------------------------------------------------------------

ConstBufDX11::ConstBufDX11()
  : buf(nullptr),
    value(nullptr),
    inst(nullptr),
    buf_i(InvalidIndex),
    regCount(0)
{
}


//------------------------------------------------------------------------------

ConstBufDX11::~ConstBufDX11()
{
}


//------------------------------------------------------------------------------

static D3D11_BLEND
_BlendOpDX11( BlendOp op )
{
    D3D11_BLEND b = D3D11_BLEND_ONE;

    switch( op )
    {
        case BLENDOP_ZERO           : b = D3D11_BLEND_ZERO; break;
        case BLENDOP_ONE            : b = D3D11_BLEND_ONE; break;
        case BLENDOP_SRC_ALPHA      : b = D3D11_BLEND_SRC_ALPHA; break;
        case BLENDOP_INV_SRC_ALPHA  : b = D3D11_BLEND_INV_SRC_ALPHA; break;
        case BLENDOP_SRC_COLOR      : b = D3D11_BLEND_SRC_COLOR; break;
        case BLENDOP_DST_COLOR      : b = D3D11_BLEND_DEST_COLOR; break;
    }
    
    return b;
}


//------------------------------------------------------------------------------

void
ConstBufDX11::Construct( ProgType ptype, unsigned bufIndex, unsigned regCnt )
{
    DVASSERT(!value);
    DVASSERT(bufIndex != InvalidIndex);
    DVASSERT(regCnt);

    D3D11_BUFFER_DESC   desc = {0};
        
    desc.ByteWidth        = regCnt*4*sizeof(float);
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags   = 0;//D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags        = 0;
        
    HRESULT hr = _D3D11_Device->CreateBuffer( &desc, NULL, &buf );

    if( SUCCEEDED(hr) )
    {
        progType = ptype;
        value    = (float*)(malloc( regCnt*4*sizeof(float) ));
        inst     = nullptr;
        buf_i    = bufIndex;
        regCount = regCnt;
    }
    else
    {
        Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D11ErrorText(hr) );
    }
}


//------------------------------------------------------------------------------

void
ConstBufDX11::Destroy()
{
    if( buf )
    {
        buf->Release();
        buf = nullptr;

        if( value )
        {
            ::free( value );
            value    = nullptr;
        }
        
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

const void*
ConstBufDX11::InstData() const
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
ConstBufDX11::InvalidateInst()
{
    inst = nullptr;
}


//------------------------------------------------------------------------------

bool
ConstBufDX11::SetConst( unsigned const_i, unsigned const_count, const float* data )
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
ConstBufDX11::SetConst( unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount )
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
ConstBufDX11::SetToRHI( const void* inst_data, ID3D11DeviceContext* context ) const
{
    context->UpdateSubresource( buf, 0, NULL, inst_data, regCount*4*sizeof(float), 0 );

    ID3D11Buffer*   cb[1] = { buf };

    if( progType == PROG_VERTEX )
        context->VSSetConstantBuffers( buf_i, 1, &buf );
    else
        context->PSSetConstantBuffers( buf_i, 1, &buf );
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

    PipelineState::Descriptor   desc;

    ID3D10Blob*         vpCode;
    ID3D11VertexShader* vertexShader;
    unsigned            vertexBufCount;
    unsigned            vertexBufRegCount[16];

    ID3D11PixelShader*  pixelShader;
    unsigned            fragmentBufCount;
    unsigned            fragmentBufRegCount[16];

    VertexLayout        vertexLayout;
    ID3D11InputLayout*  inputLayout;

    ID3D11BlendState*   blendState;


    struct
    LayoutInfo
    {
        ID3D11InputLayout*  inputLayout;
        uint32              layoutUID;
    };
    
    std::vector<LayoutInfo> altLayout;

    std::vector<uint8>      dbgVertexSrc;
    std::vector<uint8>      dbgPixelSrc;
};

typedef ResourcePool<PipelineStateDX11_t,RESOURCE_PIPELINE_STATE,PipelineState::Descriptor,false>   PipelineStateDX11Pool;
typedef ResourcePool<ConstBufDX11,RESOURCE_CONST_BUFFER,ConstBufDX11::Desc,false>                   ConstBufDX11Pool;

RHI_IMPL_POOL(PipelineStateDX11_t,RESOURCE_PIPELINE_STATE,PipelineState::Descriptor,false);
RHI_IMPL_POOL_SIZE(ConstBufDX11,RESOURCE_CONST_BUFFER,ConstBufDX11::Desc,false,8*1024);


//------------------------------------------------------------------------------

Handle
PipelineStateDX11_t::CreateConstBuffer( ProgType type, unsigned buf_i )
{
    Handle          handle = ConstBufDX11Pool::Alloc();
    ConstBufDX11*   cb     = ConstBufDX11Pool::Get( handle );

    cb->Construct( type, buf_i, (type==PROG_VERTEX) ? vertexBufRegCount[buf_i] : fragmentBufRegCount[buf_i] );

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

Logger::Info("create PS");
Logger::Info("  vprog= %s",desc.vprogUid.c_str());
Logger::Info("  fprog= %s",desc.vprogUid.c_str());
desc.vertexLayout.Dump();
    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );


    // create vertex-shader

    hr = D3DCompile
    (
        (const char*)(&vprog_bin[0]), vprog_bin.size(),
        "vprog",
        NULL, // no macros
        NULL, // no includes
        "vp_main",
        #if RHI__FORCE_DX11_91
        "vs_4_0_level_9_1",
        #else
        "vs_4_0",
        #endif
        D3DCOMPILE_OPTIMIZATION_LEVEL2,
        0, // no effect compile flags
        &vp_code,
        &vp_err
    );

    
    if( SUCCEEDED(hr) )
    {
        hr = _D3D11_Device->CreateVertexShader( vp_code->GetBufferPointer(), vp_code->GetBufferSize(), NULL, &(ps->vertexShader) );
        
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
                    ps->vertexBufCount = desc.ConstantBuffers;
                    
                    for( unsigned b=0; b!=desc.ConstantBuffers; ++b )
                    {
                        ID3D11ShaderReflectionConstantBuffer*   cb = reflection->GetConstantBufferByIndex( b );
                        
                        if( cb )
                        {
                            D3D11_SHADER_BUFFER_DESC    cb_desc;

                            hr = cb->GetDesc( &cb_desc );
                            if( SUCCEEDED(hr) )
                            {
                                ps->vertexBufRegCount[b] = cb_desc.Size/(4*sizeof(float));
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

    hr = D3DCompile
    (
        (const char*)(&fprog_bin[0]), fprog_bin.size(),
        "fprog",
        NULL, // no macros
        NULL, // no includes
        "fp_main",
        #if RHI__FORCE_DX11_91
        "ps_4_0_level_9_1",
        #else
        "ps_4_0",
        #endif
        D3DCOMPILE_OPTIMIZATION_LEVEL2,
        0, // no effect compile flags
        &fp_code,
        &fp_err
    );
    
    if( SUCCEEDED(hr) )
    {
        hr = _D3D11_Device->CreatePixelShader( fp_code->GetBufferPointer(), fp_code->GetBufferSize(), NULL, &(ps->pixelShader) );
        
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
                    ps->fragmentBufCount = desc.ConstantBuffers;

                    for( unsigned b=0; b!=desc.ConstantBuffers; ++b )
                    {
                        ID3D11ShaderReflectionConstantBuffer*   cb = reflection->GetConstantBufferByIndex( b );
                        
                        if( cb )
                        {
                            D3D11_SHADER_BUFFER_DESC    cb_desc;

                            hr = cb->GetDesc( &cb_desc );
                            if( SUCCEEDED(hr) )
                            {
                                ps->fragmentBufRegCount[b] = cb_desc.Size/(4*sizeof(float));
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
    ps->vpCode       = vp_code;
    ps->inputLayout  = _CreateInputLayout( desc.vertexLayout, vp_code->GetBufferPointer(), vp_code->GetBufferSize() );
    ps->vertexLayout = desc.vertexLayout;
    DVASSERT(ps->inputLayout);

    ps->dbgVertexSrc = vprog_bin;
    ps->dbgPixelSrc  = fprog_bin;


    // create blend-state
    
    D3D11_BLEND_DESC    bs_desc;

    bs_desc.AlphaToCoverageEnable                   = FALSE;
    bs_desc.IndependentBlendEnable                  = FALSE;
    bs_desc.RenderTarget[0].BlendEnable             = desc.blending.rtBlend[0].blendEnabled;
    bs_desc.RenderTarget[0].RenderTargetWriteMask   = D3D11_COLOR_WRITE_ENABLE_ALL;
    bs_desc.RenderTarget[0].SrcBlend                = _BlendOpDX11( BlendOp(desc.blending.rtBlend[0].colorSrc) );
    bs_desc.RenderTarget[0].DestBlend               = _BlendOpDX11( BlendOp(desc.blending.rtBlend[0].colorDst) );
    bs_desc.RenderTarget[0].BlendOp                 = D3D11_BLEND_OP_ADD;
    bs_desc.RenderTarget[0].SrcBlendAlpha           = _BlendOpDX11( BlendOp(desc.blending.rtBlend[0].alphaSrc) );
    bs_desc.RenderTarget[0].DestBlendAlpha          = _BlendOpDX11( BlendOp(desc.blending.rtBlend[0].alphaDst) );
    bs_desc.RenderTarget[0].BlendOpAlpha            = D3D11_BLEND_OP_ADD;

    hr = _D3D11_Device->CreateBlendState( &bs_desc, &(ps->blendState) );

    ps->desc = desc;

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
SetToRHI( Handle ps, uint32 layoutUID, ID3D11DeviceContext* context )
{
    PipelineStateDX11_t* ps11     = PipelineStateDX11Pool::Get( ps );
    ID3D11InputLayout*   layout11 = nullptr;

    if( layoutUID == VertexLayout::InvalidUID )
    {
        layout11 = ps11->inputLayout;
    }
    else
    {
        for( std::vector<PipelineStateDX11_t::LayoutInfo>::iterator l=ps11->altLayout.begin(),l_end=ps11->altLayout.end(); l!=l_end; ++l )
        {
            if( l->layoutUID == layoutUID )
            {
                layout11 = l->inputLayout;
                break;
            }
        }
        
        if( !layout11 )
        {
            const VertexLayout*             vbLayout = VertexLayout::Get( layoutUID );
            PipelineStateDX11_t::LayoutInfo info;
            VertexLayout                    layout;
/*            
Logger::Info("create-dx11-alt-layout:");
Logger::Info("vb-layout:");
vbLayout->Dump();
Logger::Info("vprog-layout:");
ps11->vertexLayout.Dump();
*/
            layout11 = _CreateCompatibleInputLayout( *vbLayout, ps11->vertexLayout, ps11->vpCode->GetBufferPointer(), ps11->vpCode->GetBufferSize() );

            if( layout11 )
            {
            
                info.inputLayout = layout11;
                info.layoutUID   = layoutUID;
                ps11->altLayout.push_back( info );
            }
            else
            {
                Logger::Error( "can't create compatible vertex-layout" );
                Logger::Info( "vprog-layout:" );
                ps11->vertexLayout.Dump();
                Logger::Info( "custom-layout:" );
                vbLayout->Dump();
            }
        }
    }

    context->IASetInputLayout( layout11 );
    context->VSSetShader( ps11->vertexShader, NULL, 0 );
    context->PSSetShader( ps11->pixelShader, NULL, 0 );
    context->OMSetBlendState( ps11->blendState, NULL, 0xFFFFFFFF );
}

unsigned
VertexLayoutStride( Handle ps )
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get( ps );
    
    return ps11->vertexLayout.Stride();
}

} // namespace PipelineStateDX11



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
SetToRHI( Handle cb, const void* inst_data, ID3D11DeviceContext* context )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );
    
    cb11->SetToRHI( inst_data, context );
}

const void* 
InstData( Handle cb )
{
    ConstBufDX11*   cb11 = ConstBufDX11Pool::Get( cb );
    
    return cb11->InstData();
}

void
InvalidateAllConstBufferInstances()
{
    for( ConstBufDX11Pool::Iterator b=ConstBufDX11Pool::Begin(),b_end=ConstBufDX11Pool::End(); b!=b_end; ++b )
    {
        b->InvalidateInst();
    }
}

void
InitializeRingBuffer( uint32 size )
{
    _DefConstRingBuf.Initialize( size );
}

}



} // namespace rhi