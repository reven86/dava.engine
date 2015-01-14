
    #include "../MCPP/mcpp_lib.h"

    #include "rhi_ShaderCache.h"

    

namespace rhi
{
static ShaderBuilder    _ShaderBuilder  = 0;

struct
ProgInfo
{
    DAVA::FastName      uid;
    std::vector<uint8>  bin;
};

static std::vector<ProgInfo> _ProgInfo;

static std::string*         _PreprocessedText = 0;

static int 
_mcpp__fputc( int ch, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                _PreprocessedText->push_back( (char)ch );
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }

    return ch;
}


static int
_mcpp__fputs( const char* str, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += str;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return 0;
}


static int 
_mcpp__fprintf( OUTDEST dst, const char* format, ... )
{
    va_list     arglist;
    char        buf[2048];
    int         count     = 0;

    va_start( arglist, format );
    count = vsnprintf( buf, countof(buf), format, arglist );
    va_end( arglist );

    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += buf;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return count;
}



namespace ShaderCache
{
//------------------------------------------------------------------------------

bool
Initialize( ShaderBuilder builder )
{
    _ShaderBuilder = builder;
    return true;
}


//------------------------------------------------------------------------------

void
Unitialize()
{
}


//------------------------------------------------------------------------------

void
Clear()
{
}


//------------------------------------------------------------------------------

void
Load( const char* binFileName )
{
}


//------------------------------------------------------------------------------

bool
GetProg( const DAVA::FastName& uid, std::vector<uint8>* bin )
{
    bool    success = false;

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin->clear();
            bin->insert( bin->begin(), _ProgInfo[i].bin.begin(), _ProgInfo[i].bin.end() );
            success = true;
            break;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static const char* _ShaderHeader_Metal =
"#include <metal_stdlib>\n"
"#include <metal_graphics>\n"
"#include <metal_matrix>\n"
"#include <metal_geometric>\n"
"#include <metal_math>\n"
"#include <metal_texture>\n"
"using namespace metal;\n\n"
;
    
static const char* _ShaderDefine_Metal =
"#define VPROG_IN_BEGIN          struct VP_Input {\n"
"#define VPROG_IN_POSITION       packed_float3 position; \n"
"#define VPROG_IN_NORMAL         packed_float3 normal; \n"
"#define VPROG_IN_TEXCOORD       packed_float2 texcoord; \n"
"#define VPROG_IN_END            };\n"

"#define VPROG_OUT_BEGIN         struct VP_Output {\n"
"#define VPROG_OUT_POSITION      float4 position [[ position ]]; \n"
"#define VPROG_OUT_TEXCOORD0(name,size)    float##size name [[ user(texturecoord) ]];\n"
"#define VPROG_OUT_END           };\n"

"#define DECL_VPROG_BUFFER(idx,sz) struct __VP_Buffer##idx { packed_float4 data[sz]; };\n"

"#define VPROG_BEGIN             vertex VP_Output vp_main"
"("
"    constant VP_Input*  in    [[ buffer(0) ]]"
"    VPROG_IN_BUFFER_0 "
"    VPROG_IN_BUFFER_1 "
"    ,uint                vid   [[ vertex_id ]]"
")"
"{"
"    VPROG_BUFFER_0 "
"    VPROG_BUFFER_1 "
"    VP_Output   OUT;"
"    VP_Input    IN  = in[vid];\n"

"#define VPROG_END               return OUT;"
"}\n"

"#define VP_IN_POSITION          (float3(IN.position))\n"
"#define VP_IN_NORMAL            (float3(IN.normal))\n"
"#define VP_IN_TEXCOORD          (float2(IN.texcoord))\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.name\n"


"#define FPROG_IN_BEGIN              struct FP_Input { float4 position [[position]]; \n"
"#define FPROG_IN_TEXCOORD0(name,size)    float##size name [[user(texturecoord)]];\n"
"#define FPROG_IN_END                };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color [[color(0)]];\n"
"#define FPROG_OUT_END           };\n"

"#define FP_IN(name)             IN.##name\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) struct __FP_Buffer##idx { packed_float4 data[sz]; };\n"

"#define FPROG_BEGIN             fragment FP_Output fp_main"
"("
"    FP_Input IN                 [[ stage_in ]]"
"    FPROG_IN_BUFFER_0 "
")"
"{"
"    FPROG_BUFFER_0 "
//"    const packed_float4* FP_Buffer0 = buf0->data;"
"    FP_Output   OUT;\n"
"#define FPROG_END               return OUT; }\n"

;



static const char* _ShaderHeader_GLES2 =
"#define float2                 vec2\n"
"#define float3                 vec3\n"
"#define float4                 vec4\n"
"#define float4x4               mat4\n"
"#define float3x3               mat3\n"

"vec4 mul( mat4 m, vec4 v ) { return v*m; }\n"
"vec3 mul( mat3 m, vec3 v ) { return v*m; }\n"

"float4 tex2D( sampler2D s, vec2 t ) { return texture2D(s,t); }\n"
;

static const char* _ShaderDefine_GLES2 =
"#define VPROG_IN_BEGIN          \n"
"#define VPROG_IN_POSITION       attribute vec3 attr_position;\n"
"#define VPROG_IN_NORMAL         attribute vec3 attr_normal;\n"
"#define VPROG_IN_TEXCOORD       attribute vec2 attr_texcoord;\n"
"#define VPROG_IN_END            \n"

"#define VPROG_OUT_BEGIN         \n"
"#define VPROG_OUT_POSITION      \n"
"#define VPROG_OUT_TEXCOORD0(name,size)    varying vec##size var_##name;\n"
"#define VPROG_OUT_END           \n"

"#define DECL_VPROG_BUFFER(idx,sz) uniform vec4 VP_Buffer##idx[sz];\n"

"#define VPROG_BEGIN             void main() {\n"
"#define VPROG_END               }\n"

"#define VP_IN_POSITION          attr_position\n"
"#define VP_IN_NORMAL            attr_normal\n"
"#define VP_IN_TEXCOORD          attr_texcoord\n"

"#define VP_OUT_POSITION         gl_Position\n"
"#define VP_OUT(name)            var_##name\n"


"#define FPROG_IN_BEGIN          \n"
"#define FPROG_IN_TEXCOORD0(name,size)    varying vec##size var_##name;\n"
"#define FPROG_IN_END            \n"

"#define FPROG_OUT_BEGIN         \n"
"#define FPROG_OUT_COLOR         \n"
"#define FPROG_OUT_END           \n"

"#define FP_IN(name)             var_##name\n"

"#define FP_OUT_COLOR            gl_FragColor\n"

"#define DECL_FPROG_BUFFER(idx,sz) uniform vec4 FP_Buffer##idx[sz];\n"

"#define FPROG_BEGIN             void main() {\n"
"#define FPROG_END               }\n"

;



static void
PreProcessSource( Api targetApi, const char* srcText, std::string* preprocessedText )
{

    char    src[64*1024] = "";

    switch( targetApi )
    {
        case RHI_DX11   : break;
        case RHI_DX9    : break;
        case RHI_GLES2  : 
        {
            strcat( src, _ShaderDefine_GLES2 );
        }   break;
        
        case RHI_METAL  : 
        {
            const char* s = srcText;
            const char* decl;

            while( (decl = strstr( s, "DECL_FPROG_BUFFER" )) )
            {
                int i   = 0;
                int len = strlen( src );

                sscanf( decl, "DECL_FPROG_BUFFER(%i,", &i );

                len += sprintf( src+len, "#define FPROG_IN_BUFFER_%i  ,constant __FP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, i );
                len += sprintf( src+len, "#define FPROG_BUFFER_%i    constant packed_float4* FP_Buffer%i = buf%i->data; \n", i, i, i );

                s += strlen("DECL_FPROG_BUFFER");
            }
            
            s = srcText;
            while( (decl = strstr( s, "DECL_VPROG_BUFFER" )) )
            {
                int i   = 0;
                int len = strlen( src );

                sscanf( decl, "DECL_VPROG_BUFFER(%i,", &i );

                len += sprintf( src+len, "#define VPROG_IN_BUFFER_%i  ,constant __VP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, 1+i );
                len += sprintf( src+len, "#define VPROG_BUFFER_%i    constant packed_float4* VP_Buffer%i = buf%i->data; \n", i, i, i );

                s += strlen("DECL_VPROG_BUFFER");
            }

            strcat( src, _ShaderDefine_Metal );
        }   break;
    }

    strcat( src, srcText );


    char*   argv[] = 
    { 
        "<mcpp>",   // we just need first arg
        "-P",       // do not output #line directives 
        "-C",       // keep comments
        "<input>"
    };

DAVA::Logger::Info( "src=\n%s\n", src );
    _PreprocessedText = preprocessedText;
    mcpp__set_input( src, strlen(src) );

    mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
    mcpp_lib_main( countof(argv), argv );
    _PreprocessedText = 0;
    switch( targetApi )
    {
        case RHI_DX11   : break;
        case RHI_DX9    : break;
        case RHI_GLES2  : preprocessedText->insert( 0, _ShaderHeader_GLES2 ); break;        
        case RHI_METAL  : preprocessedText->insert( 0, _ShaderHeader_Metal ); break;
    }
    ;
DAVA::Logger::Info( "pre-processed=\n%s\n", preprocessedText->c_str() );
}


//------------------------------------------------------------------------------

void
UpdateProg( Api targetApi, ProgType progType, const DAVA::FastName& uid, const char* srcText )
{
    std::string         txt;
    std::vector<uint8>* bin = 0;

    PreProcessSource( targetApi, srcText, &txt );

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if( !bin )
    {
        _ProgInfo.push_back( ProgInfo() );
        
        _ProgInfo.back().uid = uid;        
        bin = &(_ProgInfo.back().bin);
    }

    bin->clear();
    bin->insert( bin->begin(), (const uint8*)(&(txt[0])), (const uint8*)(&(txt[txt.length()-1])+1) );
    bin->push_back( 0 );
}



} // namespace ShaderCache
} // namespace rhi