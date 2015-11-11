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


    #include "MCPP/mcpp_lib.h"

    #include "../rhi_ShaderCache.h"

namespace rhi
{
static ShaderBuilder _ShaderBuilder = nullptr;

struct
ProgInfo
{
    DAVA::FastName uid;
    std::vector<uint8> bin;
};

static std::vector<ProgInfo> _ProgInfo;

static std::string* _PreprocessedText = nullptr;

static int
_mcpp__fputc(int ch, OUTDEST dst)
{
    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            _PreprocessedText->push_back((char)ch);
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return ch;
}

static int
_mcpp__fputs(const char* str, OUTDEST dst)
{
    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            *_PreprocessedText += str;
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return 0;
}

static int
_mcpp__fprintf(OUTDEST dst, const char* format, ...)
{
    va_list arglist;
    char buf[2048];
    int count = 0;

    va_start(arglist, format);
    count = vsnprintf(buf, countof(buf), format, arglist);
    va_end(arglist);

    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            *_PreprocessedText += buf;
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return count;
}

namespace ShaderCache
{
//------------------------------------------------------------------------------

bool Initialize(ShaderBuilder builder)
{
    _ShaderBuilder = builder;
    return true;
}

//------------------------------------------------------------------------------

void Unitialize()
{
}

//------------------------------------------------------------------------------

void Clear()
{
}

//------------------------------------------------------------------------------

void Load(const char* binFileName)
{
}

//------------------------------------------------------------------------------

bool GetProg(const DAVA::FastName& uid, std::vector<uint8>* bin)
{
    bool success = false;

    for (unsigned i = 0; i != _ProgInfo.size(); ++i)
    {
        if (_ProgInfo[i].uid == uid)
        {
            bin->clear();
            bin->insert(bin->begin(), _ProgInfo[i].bin.begin(), _ProgInfo[i].bin.end());
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

"#define min10float  half\n"

"#define float1 float\n"
"#define half1 half\n"
"#define min10float1 half\n"

"float4 mul( float4 v, float4x4 m );\n"
"float4 mul( float4 v, float4x4 m ) { return m*v; }\n"
"float4 mul( float4x4 m, float4 v );\n"
"float4 mul( float4x4 m, float4 v ) { return v*m; }\n"
"float3 mul( float3 v, float3x3 m );\n"
"float3 mul( float3 v, float3x3 m ) { return m*v; }\n"

"inline float  lerp( float a, float b, float t ) { return mix( a, b, t ); }\n"
"inline float2 lerp( float2 a, float2 b, float t ) { return mix( a, b, t ); }\n"
"inline float3 lerp( float3 a, float3 b, float t ) { return mix( a, b, t ); }\n"
"inline float4 lerp( float4 a, float4 b, float t ) { return mix( a, b, t ); }\n"

"#define FP_DISCARD_FRAGMENT discard_fragment()\n"
"#define FP_A8(t) t.a\n"

"#define STEP(edge,x) ((x)<(edge)) ? 0.0 : 1.0\n";

static const char* _ShaderDefine_Metal =
"#define VPROG_IN_BEGIN          struct VP_Input {\n"
"#define VPROG_IN_POSITION       float3 position [[ attribute(VATTR_POSITION) ]]; \n"
"#define VPROG_IN_NORMAL         float3 normal [[ attribute(VATTR_NORMAL) ]] ; \n"
"#define VPROG_IN_TEXCOORD       float2 texcoord0 [[ attribute(VATTR_TEXCOORD_0) ]] ; \n"
"#define VPROG_IN_TEXCOORD0(sz)  float##sz texcoord0 [[ attribute(VATTR_TEXCOORD_0) ]] ; \n"
"#define VPROG_IN_TEXCOORD1(sz)  float##sz texcoord1 [[ attribute(VATTR_TEXCOORD_1) ]] ; \n"
"#define VPROG_IN_TEXCOORD2(sz)  float##sz texcoord2 [[ attribute(VATTR_TEXCOORD_2) ]] ; \n"
"#define VPROG_IN_TEXCOORD3(sz)  float##sz texcoord3 [[ attribute(VATTR_TEXCOORD_3) ]] ; \n"
"#define VPROG_IN_TEXCOORD4(sz)  float##sz texcoord4 [[ attribute(VATTR_TEXCOORD_4) ]] ; \n"
"#define VPROG_IN_TEXCOORD5(sz)  float##sz texcoord5 [[ attribute(VATTR_TEXCOORD_5) ]] ; \n"
"#define VPROG_IN_TEXCOORD6(sz)  float##sz texcoord6 [[ attribute(VATTR_TEXCOORD_6) ]] ; \n"
"#define VPROG_IN_TEXCOORD7(sz)  float##sz texcoord7 [[ attribute(VATTR_TEXCOORD_7) ]] ; \n"
"#define VPROG_IN_COLOR          float4 color0 [[ attribute(VATTR_COLOR_0) ]] ; \n"
"#define VPROG_IN_COLOR0         float4 color0 [[ attribute(VATTR_COLOR_0) ]] ; \n"
"#define VPROG_IN_COLOR1         float4 color1 [[ attribute(VATTR_COLOR_1) ]] ; \n"
"#define VPROG_IN_TANGENT        float3 tangent [[ attribute(VATTR_TANGENT) ]] ; \n"
"#define VPROG_IN_BINORMAL       float3 binormal [[ attribute(VATTR_BINORMAL) ]] ; \n"
"#define VPROG_IN_BLENDWEIGHT    float3 blendweight [[ attribute(VATTR_BLENDWEIGHT) ]] ; \n"
"#define VPROG_IN_BLENDINDEX(sz) float##sz blendindex [[ attribute(VATTR_BLENDINDEX) ]] ; \n"
"#define VPROG_IN_END            };\n"

"#define VPROG_OUT_BEGIN                       struct VP_Output {\n"
"#define VPROG_OUT_POSITION                    float4 position [[ position ]]; \n"
"#define VPROG_OUT_TEXCOORD0(name,size)        float##size name [[ user(texcoord0) ]];\n"
"#define VPROG_OUT_TEXCOORD1(name,size)        float##size name [[ user(texcoord1) ]];\n"
"#define VPROG_OUT_TEXCOORD2(name,size)        float##size name [[ user(texcoord2) ]];\n"
"#define VPROG_OUT_TEXCOORD3(name,size)        float##size name [[ user(texcoord3) ]];\n"
"#define VPROG_OUT_TEXCOORD4(name,size)        float##size name [[ user(texcoord4) ]];\n"
"#define VPROG_OUT_TEXCOORD5(name,size)        float##size name [[ user(texcoord5) ]];\n"
"#define VPROG_OUT_TEXCOORD6(name,size)        float##size name [[ user(texcoord6) ]];\n"
"#define VPROG_OUT_TEXCOORD7(name,size)        float##size name [[ user(texcoord7) ]];\n"
"#define VPROG_OUT_TEXCOORD0_HALF(name,size)   half##size name [[ user(texcoord0) ]];\n"
"#define VPROG_OUT_TEXCOORD1_HALF(name,size)   half##size name [[ user(texcoord1) ]];\n"
"#define VPROG_OUT_TEXCOORD2_HALF(name,size)   half##size name [[ user(texcoord2) ]];\n"
"#define VPROG_OUT_TEXCOORD3_HALF(name,size)   half##size name [[ user(texcoord3) ]];\n"
"#define VPROG_OUT_TEXCOORD4_HALF(name,size)   half##size name [[ user(texcoord4) ]];\n"
"#define VPROG_OUT_TEXCOORD5_HALF(name,size)   half##size name [[ user(texcoord5) ]];\n"
"#define VPROG_OUT_TEXCOORD6_HALF(name,size)   half##size name [[ user(texcoord6) ]];\n"
"#define VPROG_OUT_TEXCOORD7_HALF(name,size)   half##size name [[ user(texcoord7) ]];\n"
"#define VPROG_OUT_TEXCOORD0_LOW(name,size)    half##size name [[ user(texcoord0) ]];\n"
"#define VPROG_OUT_TEXCOORD1_LOW(name,size)    half##size name [[ user(texcoord1) ]];\n"
"#define VPROG_OUT_TEXCOORD2_LOW(name,size)    half##size name [[ user(texcoord2) ]];\n"
"#define VPROG_OUT_TEXCOORD3_LOW(name,size)    half##size name [[ user(texcoord3) ]];\n"
"#define VPROG_OUT_TEXCOORD4_LOW(name,size)    half##size name [[ user(texcoord4) ]];\n"
"#define VPROG_OUT_TEXCOORD5_LOW(name,size)    half##size name [[ user(texcoord5) ]];\n"
"#define VPROG_OUT_TEXCOORD6_LOW(name,size)    half##size name [[ user(texcoord6) ]];\n"
"#define VPROG_OUT_TEXCOORD7_LOW(name,size)    half##size name [[ user(texcoord7) ]];\n"
"#define VPROG_OUT_COLOR0(name,size)           float##size name [[ user(color0) ]];\n"
"#define VPROG_OUT_COLOR1(name,size)           float##size name [[ user(color1) ]];\n"
"#define VPROG_OUT_COLOR0_HALF(name,size)      half##size name [[ user(color0) ]];\n"
"#define VPROG_OUT_COLOR1_HALF(name,size)      half##size name [[ user(color1) ]];\n"
"#define VPROG_OUT_COLOR0_LOW(name,size)       half##size name [[ user(color0) ]];\n"
"#define VPROG_OUT_COLOR1_LOW(name,size)       half##size name [[ user(color1) ]];\n"
"#define VPROG_OUT_END           };\n"

"#define DECL_VPROG_BUFFER(idx,sz) struct __VP_Buffer##idx { packed_float4 data[sz]; };\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( (float3)(float4(VP_Buffer##buf[reg+0])), (float3)(float4(VP_Buffer##buf[reg+1])), (float3)(float4(VP_Buffer##buf[reg+2])) );\n"

"#define VPROG_BEGIN             vertex VP_Output vp_main"
"("
"    VP_Input  in    [[ stage_in ]]"
"    VPROG_IN_BUFFER_0 "
"    VPROG_IN_BUFFER_1 "
"    VPROG_IN_BUFFER_2 "
"    VPROG_IN_BUFFER_3 "
"    VPROG_IN_BUFFER_4 "
"    VPROG_IN_BUFFER_5 "
"    VPROG_IN_BUFFER_6 "
"    VPROG_IN_BUFFER_7 "
")"
"{"
"    VPROG_BUFFER_0 "
"    VPROG_BUFFER_1 "
"    VPROG_BUFFER_2 "
"    VPROG_BUFFER_3 "
"    VPROG_BUFFER_4 "
"    VPROG_BUFFER_5 "
"    VPROG_BUFFER_6 "
"    VPROG_BUFFER_7 "
"    VP_Output   OUT;"
"    VP_Input    IN  = in;\n"

"#define VPROG_END               return OUT;"
"}\n"

"#define VP_IN_POSITION          (float3(IN.position))\n"
"#define VP_IN_NORMAL            (float3(IN.normal))\n"
"#define VP_IN_TEXCOORD          (float2(IN.texcoord0))\n"
"#define VP_IN_TEXCOORD0         IN.texcoord0\n"
"#define VP_IN_TEXCOORD1         IN.texcoord1\n"
"#define VP_IN_TEXCOORD2         IN.texcoord2\n"
"#define VP_IN_TEXCOORD3         IN.texcoord3\n"
"#define VP_IN_TEXCOORD4         IN.texcoord4\n"
"#define VP_IN_TEXCOORD5         IN.texcoord5\n"
"#define VP_IN_TEXCOORD6         IN.texcoord6\n"
"#define VP_IN_TEXCOORD7         IN.texcoord7\n"
"#define VP_IN_COLOR             (float4(IN.color0[0],IN.color0[1],IN.color0[2],IN.color0[3]))\n"
"#define VP_IN_COLOR0            (float4(IN.color0[0],IN.color0[1],IN.color0[2],IN.color0[3]))\n"
"#define VP_IN_COLOR1            (float4(IN.color1[0],IN.color1[1],IN.color1[2],IN.color1[3]))\n"
"#define VP_IN_TANGENT           (float3(IN.tangent))\n"
"#define VP_IN_BINORMAL          (float3(IN.binormal))\n"
"#define VP_IN_BLENDWEIGHT       (float3(IN.blendweight))\n"
"#define VP_IN_BLENDINDEX        (IN.blendindex)\n"

"#define VP_TEXTURE2D(unit,uv)   tex##unit.sample( tex##unit##_sampler, uv, level(0) )\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.name\n"

"#define FPROG_IN_BEGIN                        struct FP_Input { float4 position [[position]]; \n"
"#define FPROG_IN_TEXCOORD0(name,size)         float##size name [[ user(texcoord0) ]];\n"
"#define FPROG_IN_TEXCOORD1(name,size)         float##size name [[ user(texcoord1) ]];\n"
"#define FPROG_IN_TEXCOORD2(name,size)         float##size name [[ user(texcoord2) ]];\n"
"#define FPROG_IN_TEXCOORD3(name,size)         float##size name [[ user(texcoord3) ]];\n"
"#define FPROG_IN_TEXCOORD4(name,size)         float##size name [[ user(texcoord4) ]];\n"
"#define FPROG_IN_TEXCOORD5(name,size)         float##size name [[ user(texcoord5) ]];\n"
"#define FPROG_IN_TEXCOORD6(name,size)         float##size name [[ user(texcoord6) ]];\n"
"#define FPROG_IN_TEXCOORD7(name,size)         float##size name [[ user(texcoord7) ]];\n"
"#define FPROG_IN_TEXCOORD0_HALF(name,size)    half##size name [[ user(texcoord0) ]];\n"
"#define FPROG_IN_TEXCOORD1_HALF(name,size)    half##size name [[ user(texcoord1) ]];\n"
"#define FPROG_IN_TEXCOORD2_HALF(name,size)    half##size name [[ user(texcoord2) ]];\n"
"#define FPROG_IN_TEXCOORD3_HALF(name,size)    half##size name [[ user(texcoord3) ]];\n"
"#define FPROG_IN_TEXCOORD4_HALF(name,size)    half##size name [[ user(texcoord4) ]];\n"
"#define FPROG_IN_TEXCOORD5_HALF(name,size)    half##size name [[ user(texcoord5) ]];\n"
"#define FPROG_IN_TEXCOORD6_HALF(name,size)    half##size name [[ user(texcoord6) ]];\n"
"#define FPROG_IN_TEXCOORD7_HALF(name,size)    half##size name [[ user(texcoord7) ]];\n"
"#define FPROG_IN_TEXCOORD0_LOW(name,size)     half##size name [[ user(texcoord0) ]];\n"
"#define FPROG_IN_TEXCOORD1_LOW(name,size)     half##size name [[ user(texcoord1) ]];\n"
"#define FPROG_IN_TEXCOORD2_LOW(name,size)     half##size name [[ user(texcoord2) ]];\n"
"#define FPROG_IN_TEXCOORD3_LOW(name,size)     half##size name [[ user(texcoord3) ]];\n"
"#define FPROG_IN_TEXCOORD4_LOW(name,size)     half##size name [[ user(texcoord4) ]];\n"
"#define FPROG_IN_TEXCOORD5_LOW(name,size)     half##size name [[ user(texcoord5) ]];\n"
"#define FPROG_IN_TEXCOORD6_LOW(name,size)     half##size name [[ user(texcoord6) ]];\n"
"#define FPROG_IN_TEXCOORD7_LOW(name,size)     half##size name [[ user(texcoord7) ]];\n"
"#define FPROG_IN_COLOR0(name,size)            float##size name [[ user(color0) ]];\n"
"#define FPROG_IN_COLOR1(name,size)            float##size name [[ user(color1) ]];\n"
"#define FPROG_IN_COLOR0_HALF(name,size)       half##size name [[ user(color0) ]];\n"
"#define FPROG_IN_COLOR1_HALF(name,size)       half##size name [[ user(color1) ]];\n"
"#define FPROG_IN_COLOR0_LOW(name,size)        half##size name [[ user(color0) ]];\n"
"#define FPROG_IN_COLOR1_LOW(name,size)        half##size name [[ user(color1) ]];\n"
"#define FPROG_IN_END                          };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color [[color(0)]];\n"
"#define FPROG_OUT_END           };\n"

"#define DECL_FP_SAMPLER2D(unit)    \n"
"#define DECL_FP_SAMPLERCUBE(unit)  \n"

"#define DECL_VP_SAMPLER2D(unit)    \n"

"#define FP_TEXTURE2D(unit,uv)   fp_tex##unit.sample( fp_tex##unit##_sampler, uv )\n"
"#define FP_TEXTURECUBE(unit,uv) fp_tex##unit.sample( fp_tex##unit##_sampler, uv )\n"
"#define FP_IN(name)             IN.##name\n"

"#define VP_TEXTURE2D(unit,uv)   vp_tex##unit.sample( vp_tex##unit##_sampler, uv )\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) struct __FP_Buffer##idx { packed_float4 data[sz]; };\n"

"#define FPROG_BEGIN             fragment FP_Output fp_main"
"("
"    FP_Input IN                 [[ stage_in ]]"
"    FPROG_IN_BUFFER_0 "
"    FPROG_IN_BUFFER_1 "
"    FPROG_IN_BUFFER_2 "
"    FPROG_IN_BUFFER_3 "
"    FPROG_IN_BUFFER_4 "
"    FPROG_IN_BUFFER_5 "
"    FPROG_IN_BUFFER_6 "
"    FPROG_IN_BUFFER_7 "
"    FPROG_IN_TEXTURE_0 "
"    FPROG_IN_TEXTURE_1 "
"    FPROG_IN_TEXTURE_2 "
"    FPROG_IN_TEXTURE_3 "
"    FPROG_IN_TEXTURE_4 "
"    FPROG_IN_TEXTURE_5 "
"    FPROG_IN_TEXTURE_6 "
"    FPROG_IN_TEXTURE_7 "
"    FPROG_SAMPLER_0 "
"    FPROG_SAMPLER_1 "
"    FPROG_SAMPLER_2 "
"    FPROG_SAMPLER_3 "
"    FPROG_SAMPLER_4 "
"    FPROG_SAMPLER_5 "
"    FPROG_SAMPLER_6 "
"    FPROG_SAMPLER_7 "
")"
"{"
"    FPROG_BUFFER_0 "
"    FPROG_BUFFER_1 "
"    FPROG_BUFFER_2 "
"    FPROG_BUFFER_3 "
"    FPROG_BUFFER_4 "
"    FPROG_BUFFER_5 "
"    FPROG_BUFFER_6 "
"    FPROG_BUFFER_7 "
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
"#define vec1                   float\n"
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
"#define half                   mediump float\n"
"#define half1                  mediump float\n"
"#define half2                  mediump vec2\n"
"#define half3                  mediump vec3\n"
"#define half4                  mediump vec4\n"
"#define min10float             lowp float\n"
"#define min10float1            lowp float\n"
"#define min10float2            lowp vec2\n"
"#define min10float3            lowp vec3\n"
"#define min10float4            lowp vec4\n"
#else
"#define half                   float\n"
"#define half1                  float\n"
"#define half2                  vec2\n"
"#define half3                  vec3\n"
"#define half4                  vec4\n"
"#define min10float             float\n"
"#define min10float1            float\n"
"#define min10float2            vec2\n"
"#define min10float3            vec3\n"
"#define min10float4            vec4\n"
#endif
//"vec4 mul( vec4 v, mat4 m ) { return m*v; }\n"
//"vec4 mul( mat4 m, vec4 v ) { return v*m; }\n"
//"vec3 mul( vec3 v, mat3 m ) { return m*v; }\n"
"#define mul( v, m ) ((m)*(v))\n"

"#define lerp(a,b,t) mix( (a), (b), (t) )\n"

"#define FP_DISCARD_FRAGMENT discard\n"
"#define FP_A8(t) t.a\n"

"#define STEP(edge,x) step( (edge), (x) )\n";

static const char* _ShaderDefine_GLES2 =
"#define VPROG_IN_BEGIN          \n"
"#define VPROG_IN_POSITION       attribute vec3 attr_position;\n"
"#define VPROG_IN_NORMAL         attribute vec3 attr_normal;\n"
"#define VPROG_IN_TEXCOORD       attribute vec2 attr_texcoord0;\n"
"#define VPROG_IN_TEXCOORD0(sz)  attribute vec##sz attr_texcoord0;\n"
"#define VPROG_IN_TEXCOORD1(sz)  attribute vec##sz attr_texcoord1;\n"
"#define VPROG_IN_TEXCOORD2(sz)  attribute vec##sz attr_texcoord2;\n"
"#define VPROG_IN_TEXCOORD3(sz)  attribute vec##sz attr_texcoord3;\n"
"#define VPROG_IN_TEXCOORD4(sz)  attribute vec##sz attr_texcoord4;\n"
"#define VPROG_IN_TEXCOORD5(sz)  attribute vec##sz attr_texcoord5;\n"
"#define VPROG_IN_TEXCOORD6(sz)  attribute vec##sz attr_texcoord6;\n"
"#define VPROG_IN_TEXCOORD7(sz)  attribute vec##sz attr_texcoord7;\n"
"#define VPROG_IN_COLOR          attribute vec4 attr_color0;\n"
"#define VPROG_IN_COLOR0         attribute vec4 attr_color0;\n"
"#define VPROG_IN_COLOR1         attribute vec4 attr_color1;\n"
"#define VPROG_IN_TANGENT        attribute vec3 attr_tangent;\n"
"#define VPROG_IN_BINORMAL       attribute vec3 attr_binormal;\n"
"#define VPROG_IN_BLENDWEIGHT    attribute vec3 attr_blendweight;\n"
"#define VPROG_IN_BLENDINDEX(sz) attribute vec##sz attr_blendindex;\n"
"#define VPROG_IN_END            \n"

"#define VPROG_OUT_BEGIN         \n"
"#define VPROG_OUT_POSITION      \n"
"#define VPROG_OUT_TEXCOORD0(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD1(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD2(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD3(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD4(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD5(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD6(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD7(name,size)         varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD0_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD1_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD2_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD3_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD4_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD5_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD6_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD7_HALF(name,size)    varying half##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD0_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD1_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD2_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD3_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD4_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD5_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD6_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD7_LOW(name,size)     varying min10float##size var_##name;\n"
"#define VPROG_OUT_COLOR0(name,size)            varying vec##size var_##name;\n"
"#define VPROG_OUT_COLOR1(name,size)            varying vec##size var_##name;\n"
"#define VPROG_OUT_COLOR0_HALF(name,size)       varying half##size var_##name;\n"
"#define VPROG_OUT_COLOR1_HALF(name,size)       varying half##size var_##name;\n"
"#define VPROG_OUT_COLOR0_LOW(name,size)        varying min10float##size var_##name;\n"
"#define VPROG_OUT_COLOR1_LOW(name,size)        varying min10float##size var_##name;\n"
"#define VPROG_OUT_END           \n"

"#define DECL_VPROG_BUFFER(idx,sz) uniform vec4 VP_Buffer##idx[sz];\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( float3(VP_Buffer##buf[reg+0]), float3(VP_Buffer##buf[reg+1]), float3(VP_Buffer##buf[reg+2]) );\n"

"#define VPROG_BEGIN             void main() {\n"
"#define VPROG_END               }\n"

"#define VP_IN_POSITION          attr_position\n"
"#define VP_IN_NORMAL            attr_normal\n"
"#define VP_IN_TEXCOORD          attr_texcoord0\n"
"#define VP_IN_TEXCOORD0         attr_texcoord0\n"
"#define VP_IN_TEXCOORD1         attr_texcoord1\n"
"#define VP_IN_TEXCOORD2         attr_texcoord2\n"
"#define VP_IN_TEXCOORD3         attr_texcoord3\n"
"#define VP_IN_TEXCOORD4         attr_texcoord4\n"
"#define VP_IN_TEXCOORD5         attr_texcoord5\n"
"#define VP_IN_TEXCOORD6         attr_texcoord6\n"
"#define VP_IN_TEXCOORD7         attr_texcoord7\n"
"#define VP_IN_COLOR             attr_color0\n"
"#define VP_IN_COLOR0            attr_color0\n"
"#define VP_IN_COLOR1            attr_color1\n"
"#define VP_IN_TANGENT           attr_tangent\n"
"#define VP_IN_BINORMAL          attr_binormal\n"
"#define VP_IN_BLENDWEIGHT       attr_blendweigh\n"
"#define VP_IN_BLENDINDEX        attr_blendindex\n"

"#define VP_OUT_POSITION         gl_Position\n"
"#define VP_OUT(name)            var_##name\n"

"#define VP_TEXTURE2D(unit,uv)   texture2DLod( VertexTexture##unit, uv, 0.0 )\n"

"#define FPROG_IN_BEGIN          \n"
"#define FPROG_IN_TEXCOORD0(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD1(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD2(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD3(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD4(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD5(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD6(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD7(name,size)         varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD0_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD1_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD2_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD3_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD4_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD5_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD6_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD7_HALF(name,size)    varying half##size var_##name;\n"
"#define FPROG_IN_TEXCOORD0_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD1_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD2_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD3_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD4_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD5_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD6_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_TEXCOORD7_LOW(name,size)     varying min10float##size var_##name;\n"
"#define FPROG_IN_COLOR0(name,size)            varying float##size var_##name;\n"
"#define FPROG_IN_COLOR1(name,size)            varying float##size var_##name;\n"
"#define FPROG_IN_COLOR0_HALF(name,size)       varying half##size var_##name;\n"
"#define FPROG_IN_COLOR1_HALF(name,size)       varying half##size var_##name;\n"
"#define FPROG_IN_COLOR0_LOW(name,size)        varying min10float##size var_##name;\n"
"#define FPROG_IN_COLOR1_LOW(name,size)        varying min10float##size var_##name;\n"
"#define FPROG_IN_END            \n"

"#define FPROG_OUT_BEGIN         \n"
"#define FPROG_OUT_COLOR         \n"
"#define FPROG_OUT_END           \n"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
"#define DECL_FP_SAMPLER2D(unit)    uniform lowp sampler2D FragmentTexture##unit;\n"
"#define DECL_FP_SAMPLERCUBE(unit)  uniform lowp samplerCube FragmentTexture##unit;\n"
"#define DECL_VP_SAMPLER2D(unit)    uniform lowp sampler2D VertexTexture##unit;\n"
#else
"#define DECL_FP_SAMPLER2D(unit)    uniform sampler2D FragmentTexture##unit;\n"
"#define DECL_FP_SAMPLERCUBE(unit)  uniform samplerCube FragmentTexture##unit;\n"
"#define DECL_VP_SAMPLER2D(unit)    uniform sampler2D VertexTexture##unit;\n"
#endif

"#define FP_TEXTURE2D(unit,uv)   texture2D( FragmentTexture##unit, uv )\n"
"#define FP_TEXTURECUBE(unit,uv) textureCube( FragmentTexture##unit, uv )\n"
"#define FP_IN(name)             var_##name\n"

"#define FP_OUT_COLOR            gl_FragColor\n"

"#define DECL_FPROG_BUFFER(idx,sz) uniform vec4 FP_Buffer##idx[sz];\n"

"#define FPROG_BEGIN             void main() {\n"
"#define FPROG_END               }\n"

;

static const char* _ShaderHeader_DX9 =
"#define float1                 float\n"
"#define half1                  half\n"
"#define min10float             half\n"
"#define min10float1            half\n"
"#define min10float2            half2\n"
"#define min10float3            half3\n"
"#define min10float4            half4\n"

"#define FP_DISCARD_FRAGMENT discard\n"
"#define FP_A8(t) t.a\n"

"#define STEP(edge,x) step( (edge), (x) )\n";

static const char* _ShaderDefine_DX9 =
"#define VPROG_IN_BEGIN                 struct VP_Input {\n"
"#define VPROG_IN_POSITION              float3 position : POSITION0;\n"
"#define VPROG_IN_NORMAL                float3 normal : NORMAL0;\n"
"#define VPROG_IN_TEXCOORD              float2 texcoord0 : TEXCOORD0;\n"
"#define VPROG_IN_TEXCOORD0(sz)         float##sz texcoord0 : TEXCOORD0;\n"
"#define VPROG_IN_TEXCOORD1(sz)         float##sz texcoord1 : TEXCOORD1;\n"
"#define VPROG_IN_TEXCOORD2(sz)         float##sz texcoord2 : TEXCOORD2;\n"
"#define VPROG_IN_TEXCOORD3(sz)         float##sz texcoord3 : TEXCOORD3;\n"
"#define VPROG_IN_TEXCOORD4(sz)         float##sz texcoord4 : TEXCOORD4;\n"
"#define VPROG_IN_TEXCOORD5(sz)         float##sz texcoord5 : TEXCOORD5;\n"
"#define VPROG_IN_TEXCOORD6(sz)         float##sz texcoord6 : TEXCOORD6;\n"
"#define VPROG_IN_TEXCOORD7(sz)         float##sz texcoord7 : TEXCOORD7;\n"
"#define VPROG_IN_COLOR                 float4 color0 : COLOR0;\n"
"#define VPROG_IN_COLOR0                float4 color0 : COLOR0;\n"
"#define VPROG_IN_COLOR1                float4 color1 : COLOR1;\n"
"#define VPROG_IN_TANGENT               float3 tangent : TANGENT;\n"
"#define VPROG_IN_BINORMAL              float3 binormal : BINORMAL;\n"
"#define VPROG_IN_BLENDWEIGHT           float3 blendweight : BLENDWEIGHT;\n"
"#define VPROG_IN_BLENDINDEX(sz)        float##sz blendindex : BLENDINDICES;\n"
"#define VPROG_IN_END                   };\n"

"#define VPROG_OUT_BEGIN                        struct VP_Output {\n"
"#define VPROG_OUT_POSITION                     float4 position : POSITION0;\n"
"#define VPROG_OUT_TEXCOORD0(name,size)         float##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1(name,size)         float##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2(name,size)         float##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3(name,size)         float##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4(name,size)         float##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5(name,size)         float##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6(name,size)         float##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7(name,size)         float##size name : TEXCOORD7;\n"
"#define VPROG_OUT_TEXCOORD0_HALF(name,size)    half##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1_HALF(name,size)    half##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2_HALF(name,size)    half##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3_HALF(name,size)    half##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4_HALF(name,size)    half##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5_HALF(name,size)    half##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6_HALF(name,size)    half##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7_HALF(name,size)    half##size name : TEXCOORD7;\n"
"#define VPROG_OUT_TEXCOORD0_LOW(name,size)     half##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1_LOW(name,size)     half##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2_LOW(name,size)     half##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3_LOW(name,size)     half##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4_LOW(name,size)     half##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5_LOW(name,size)     half##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6_LOW(name,size)     half##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7_LOW(name,size)     half##size name : TEXCOORD7;\n"
"#define VPROG_OUT_COLOR0(name,size)            float##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1(name,size)            float##size name : COLOR1;\n"
"#define VPROG_OUT_COLOR0_HALF(name,size)       half##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1_HALF(name,size)       half##size name : COLOR1;\n"
"#define VPROG_OUT_COLOR0_LOW(name,size)        half##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1_LOW(name,size)        half##size name : COLOR1;\n"
"#define VPROG_OUT_END                          };\n"

"#define DECL_VPROG_BUFFER(idx,sz) uniform float4 VP_Buffer##idx[sz];\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( (float3)(float4(VP_Buffer##buf[reg+0])), (float3)(float4(VP_Buffer##buf[reg+1])), (float3)(float4(VP_Buffer##buf[reg+2])) );\n"

"#define VPROG_BEGIN             VP_Output vp_main( VP_Input IN ) { VP_Output OUT;\n"
"#define VPROG_END               return OUT; }\n"

"#define VP_IN_POSITION          IN.position\n"
"#define VP_IN_NORMAL            IN.normal\n"
"#define VP_IN_TEXCOORD          IN.texcoord0\n"
"#define VP_IN_TEXCOORD0         IN.texcoord0\n"
"#define VP_IN_TEXCOORD1         IN.texcoord1\n"
"#define VP_IN_TEXCOORD2         IN.texcoord2\n"
"#define VP_IN_TEXCOORD3         IN.texcoord3\n"
"#define VP_IN_TEXCOORD4         IN.texcoord4\n"
"#define VP_IN_TEXCOORD5         IN.texcoord5\n"
"#define VP_IN_TEXCOORD6         IN.texcoord6\n"
"#define VP_IN_TEXCOORD7         IN.texcoord7\n"
"#define VP_IN_COLOR             IN.color0\n"
"#define VP_IN_COLOR0            IN.color0\n"
"#define VP_IN_COLOR1            IN.color1\n"
"#define VP_IN_TANGENT           IN.tangent\n"
"#define VP_IN_BINORMAL          IN.binormal\n"
"#define VP_IN_BLENDWEIGHT       IN.blendweight\n"
"#define VP_IN_BLENDINDEX        IN.blendindex\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.##name\n"

"#define VP_TEXTURE2D(unit,uv)   tex2Dlod( VertexTexture##unit, float4(uv.x,uv.y,0,0) )\n"

"#define FPROG_IN_BEGIN                        struct FP_Input {\n"
"#define FPROG_IN_TEXCOORD0(name,size)         float##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1(name,size)         float##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2(name,size)         float##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3(name,size)         float##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4(name,size)         float##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5(name,size)         float##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6(name,size)         float##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7(name,size)         float##size name : TEXCOORD7;\n"
"#define FPROG_IN_TEXCOORD0_HALF(name,size)    half##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1_HALF(name,size)    half##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2_HALF(name,size)    half##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3_HALF(name,size)    half##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4_HALF(name,size)    half##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5_HALF(name,size)    half##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6_HALF(name,size)    half##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7_HALF(name,size)    half##size name : TEXCOORD7;\n"
"#define FPROG_IN_TEXCOORD0_LOW(name,size)     half##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1_LOW(name,size)     half##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2_LOW(name,size)     half##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3_LOW(name,size)     half##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4_LOW(name,size)     half##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5_LOW(name,size)     half##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6_LOW(name,size)     half##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7_LOW(name,size)     half##size name : TEXCOORD7;\n"
"#define FPROG_IN_COLOR0(name,size)            float##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1(name,size)            float##size name : COLOR1;\n"
"#define FPROG_IN_COLOR0_HALF(name,size)       half##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1_HALF(name,size)       half##size name : COLOR1;\n"
"#define FPROG_IN_COLOR0_LOW(name,size)        half##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1_LOW(name,size)        half##size name : COLOR1;\n"
"#define FPROG_IN_END                          };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color : COLOR0;\n"
"#define FPROG_OUT_END           };\n"

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
"#define DECL_FP_SAMPLER2D(unit)    uniform lowp sampler2D FragmentTexture##unit : TEXUNIT##unit;\n"
"#define DECL_FP_SAMPLERCUBE(unit)  uniform lowp samplerCUBE FragmentTexture##unit : TEXUNIT##unit;\n"
"#define DECL_VP_SAMPLER2D(unit)    uniform lowp sampler2D VertexTexture##unit : TEXUNIT##unit;\n"
#else
"#define DECL_FP_SAMPLER2D(unit)    uniform sampler2D FragmentTexture##unit : TEXUNIT##unit;\n"
"#define DECL_FP_SAMPLERCUBE(unit)  uniform samplerCUBE FragmentTexture##unit : TEXUNIT##unit;\n"
"#define DECL_VP_SAMPLER2D(unit)    uniform sampler2D VertexTexture##unit : TEXUNIT##unit;\n"
#endif

"#define FP_TEXTURE2D(unit,uv)   tex2D( FragmentTexture##unit, uv )\n"
"#define FP_TEXTURECUBE(unit,uv) texCUBE( FragmentTexture##unit, uv )\n"
"#define FP_IN(name)             IN.##name\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) uniform float4 FP_Buffer##idx[sz];\n"

"#define FPROG_BEGIN             FP_Output fp_main( FP_Input IN ) { FP_Output OUT;\n"
"#define FPROG_END               return OUT; }\n"

;

static const char* _ShaderHeader_DX11 =
"#define float1                 float\n"
"#define half1                  half\n"

"#define FP_DISCARD_FRAGMENT discard\n"
"#define FP_A8(t) t.r\n"

"#define STEP(edge,x) step( (edge), (x) )\n";

static const char* _ShaderDefine_DX11 =
"#define VPROG_IN_BEGIN                 struct VP_Input {\n"
"#define VPROG_IN_POSITION              float3 position : POSITION;\n"
"#define VPROG_IN_NORMAL                float3 normal : NORMAL;\n"
"#define VPROG_IN_TEXCOORD              float2 texcoord0 : TEXCOORD0;\n"
"#define VPROG_IN_TEXCOORD0(sz)         float##sz texcoord0 : TEXCOORD0;\n"
"#define VPROG_IN_TEXCOORD1(sz)         float##sz texcoord1 : TEXCOORD1;\n"
"#define VPROG_IN_TEXCOORD2(sz)         float##sz texcoord2 : TEXCOORD2;\n"
"#define VPROG_IN_TEXCOORD3(sz)         float##sz texcoord3 : TEXCOORD3;\n"
"#define VPROG_IN_TEXCOORD4(sz)         float##sz texcoord4 : TEXCOORD4;\n"
"#define VPROG_IN_TEXCOORD5(sz)         float##sz texcoord5 : TEXCOORD5;\n"
"#define VPROG_IN_TEXCOORD6(sz)         float##sz texcoord6 : TEXCOORD6;\n"
"#define VPROG_IN_TEXCOORD7(sz)         float##sz texcoord7 : TEXCOORD7;\n"
"#define VPROG_IN_COLOR                 float4 color0 : COLOR0;\n"
"#define VPROG_IN_COLOR0                float4 color0 : COLOR0;\n"
"#define VPROG_IN_COLOR1                float4 color1 : COLOR1;\n"
"#define VPROG_IN_TANGENT               float3 tangent : TANGENT;\n"
"#define VPROG_IN_BINORMAL              float3 binormal : BINORMAL;\n"
"#define VPROG_IN_BLENDWEIGHT           float3 blendweight : BLENDWEIGHT;\n"
"#define VPROG_IN_BLENDINDEX(sz)        float##sz blendindex : BLENDINDICES;\n"
"#define VPROG_IN_END                   };\n"

"#define VPROG_OUT_BEGIN                        struct VP_Output {\n"
"#define VPROG_OUT_POSITION                     float4 position : SV_POSITION;\n"
"#define VPROG_OUT_TEXCOORD0(name,size)         float##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1(name,size)         float##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2(name,size)         float##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3(name,size)         float##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4(name,size)         float##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5(name,size)         float##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6(name,size)         float##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7(name,size)         float##size name : TEXCOORD7;\n"
"#define VPROG_OUT_TEXCOORD0_HALF(name,size)    half##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1_HALF(name,size)    half##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2_HALF(name,size)    half##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3_HALF(name,size)    half##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4_HALF(name,size)    half##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5_HALF(name,size)    half##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6_HALF(name,size)    half##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7_HALF(name,size)    half##size name : TEXCOORD7;\n"
"#define VPROG_OUT_TEXCOORD0_LOW(name,size)     half##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1_LOW(name,size)     half##size name : TEXCOORD1;\n"
"#define VPROG_OUT_TEXCOORD2_LOW(name,size)     half##size name : TEXCOORD2;\n"
"#define VPROG_OUT_TEXCOORD3_LOW(name,size)     half##size name : TEXCOORD3;\n"
"#define VPROG_OUT_TEXCOORD4_LOW(name,size)     half##size name : TEXCOORD4;\n"
"#define VPROG_OUT_TEXCOORD5_LOW(name,size)     half##size name : TEXCOORD5;\n"
"#define VPROG_OUT_TEXCOORD6_LOW(name,size)     half##size name : TEXCOORD6;\n"
"#define VPROG_OUT_TEXCOORD7_LOW(name,size)     half##size name : TEXCOORD7;\n"
"#define VPROG_OUT_COLOR0(name,size)            float##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1(name,size)            float##size name : COLOR1;\n"
"#define VPROG_OUT_COLOR0_HALF(name,size)       half##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1_HALF(name,size)       half##size name : COLOR1;\n"
"#define VPROG_OUT_COLOR0_LOW(name,size)        half##size name : COLOR0;\n"
"#define VPROG_OUT_COLOR1_LOW(name,size)        half##size name : COLOR1;\n"
"#define VPROG_OUT_END                          };\n"

//"#define DECL_VPROG_BUFFER(idx,sz) uniform float4 VP_Buffer##idx[sz];\n"
"#define DECL_VPROG_BUFFER(idx,sz) cbuffer VP_Buffer##idx##_t : register(b##idx) { float4 VP_Buffer##idx[sz]; };\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( (float3)(float4(VP_Buffer##buf[reg+0])), (float3)(float4(VP_Buffer##buf[reg+1])), (float3)(float4(VP_Buffer##buf[reg+2])) );\n"

"#define VPROG_BEGIN             VP_Output vp_main( VP_Input IN ) { VP_Output OUT;\n"
"#define VPROG_END               return OUT; }\n"

"#define VP_IN_POSITION          IN.position\n"
"#define VP_IN_NORMAL            IN.normal\n"
"#define VP_IN_TEXCOORD          IN.texcoord0\n"
"#define VP_IN_TEXCOORD0         IN.texcoord0\n"
"#define VP_IN_TEXCOORD1         IN.texcoord1\n"
"#define VP_IN_TEXCOORD2         IN.texcoord2\n"
"#define VP_IN_TEXCOORD3         IN.texcoord3\n"
"#define VP_IN_TEXCOORD4         IN.texcoord4\n"
"#define VP_IN_TEXCOORD5         IN.texcoord5\n"
"#define VP_IN_TEXCOORD6         IN.texcoord6\n"
"#define VP_IN_TEXCOORD7         IN.texcoord7\n"
"#define VP_IN_COLOR             IN.color0\n"
"#define VP_IN_COLOR0            IN.color0\n"
"#define VP_IN_COLOR1            IN.color1\n"
"#define VP_IN_TANGENT           IN.tangent\n"
"#define VP_IN_BINORMAL          IN.binormal\n"
"#define VP_IN_BLENDWEIGHT       IN.blendweight\n"
"#define VP_IN_BLENDINDEX        IN.blendindex\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.##name\n"

//"#define VP_TEXTURE2D(unit,uv)   tex2Dlod( VertexTexture##unit, float4(uv.x,uv.y,0,0) )\n"
"#define VP_TEXTURE2D(unit,uv)   VertexTexture##unit.SampleLevel( VertexTexture##unit##_Sampler, uv, 0 )\n"

"#define FPROG_IN_BEGIN                        struct FP_Input { float4 pos : SV_POSITION; \n"
"#define FPROG_IN_TEXCOORD0(name,size)         float##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1(name,size)         float##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2(name,size)         float##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3(name,size)         float##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4(name,size)         float##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5(name,size)         float##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6(name,size)         float##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7(name,size)         float##size name : TEXCOORD7;\n"
"#define FPROG_IN_TEXCOORD0_HALF(name,size)    half##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1_HALF(name,size)    half##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2_HALF(name,size)    half##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3_HALF(name,size)    half##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4_HALF(name,size)    half##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5_HALF(name,size)    half##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6_HALF(name,size)    half##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7_HALF(name,size)    half##size name : TEXCOORD7;\n"
"#define FPROG_IN_TEXCOORD0_LOW(name,size)     half##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1_LOW(name,size)     half##size name : TEXCOORD1;\n"
"#define FPROG_IN_TEXCOORD2_LOW(name,size)     half##size name : TEXCOORD2;\n"
"#define FPROG_IN_TEXCOORD3_LOW(name,size)     half##size name : TEXCOORD3;\n"
"#define FPROG_IN_TEXCOORD4_LOW(name,size)     half##size name : TEXCOORD4;\n"
"#define FPROG_IN_TEXCOORD5_LOW(name,size)     half##size name : TEXCOORD5;\n"
"#define FPROG_IN_TEXCOORD6_LOW(name,size)     half##size name : TEXCOORD6;\n"
"#define FPROG_IN_TEXCOORD7_LOW(name,size)     half##size name : TEXCOORD7;\n"
"#define FPROG_IN_COLOR0(name,size)            float##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1(name,size)            float##size name : COLOR1;\n"
"#define FPROG_IN_COLOR0_HALF(name,size)       half##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1_HALF(name,size)       half##size name : COLOR1;\n"
"#define FPROG_IN_COLOR0_LOW(name,size)        half##size name : COLOR0;\n"
"#define FPROG_IN_COLOR1_LOW(name,size)        half##size name : COLOR1;\n"
"#define FPROG_IN_END                          };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color : SV_TARGET0;\n"
"#define FPROG_OUT_END           };\n"

"#define DECL_FP_SAMPLER2D(unit)    Texture2D FragmentTexture##unit : register(t##unit); SamplerState FragmentTexture##unit##_Sampler : register(s##unit);\n"
"#define DECL_FP_SAMPLERCUBE(unit)    TextureCube FragmentTexture##unit : register(t##unit); SamplerState FragmentTexture##unit##_Sampler : register(s##unit);\n"
"#define DECL_VP_SAMPLER2D(unit)    Texture2D VertexTexture##unit : register(t##unit); SamplerState VertexTexture##unit##_Sampler : register(s##unit);\n"

"#define FP_TEXTURE2D(unit,uv)   FragmentTexture##unit.Sample( FragmentTexture##unit##_Sampler, uv )\n"
"#define FP_TEXTURECUBE(unit,uv) FragmentTexture##unit.Sample( FragmentTexture##unit##_Sampler, uv )\n"
"#define FP_IN(name)             IN.##name\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) cbuffer FP_Buffer##idx##_t : register(b##idx) { float4 FP_Buffer##idx[sz]; };\n"

"#define FPROG_BEGIN             FP_Output fp_main( FP_Input IN ) { FP_Output OUT;\n"
"#define FPROG_END               return OUT; }\n"

;

static void
PreProcessSource(Api targetApi, const char* srcText, std::string* preprocessedText)
{
    char src[256 * 1024] = "";
    int src_len = 0;

    // inject vattr definitions
    {
        struct
        {
            const char* name;
            int value;
        } vattr[] =
        {
          { "VATTR_POSITION", VATTR_POSITION },
          { "VATTR_NORMAL", VATTR_NORMAL },
          { "VATTR_TEXCOORD_0", VATTR_TEXCOORD_0 },
          { "VATTR_TEXCOORD_1", VATTR_TEXCOORD_1 },
          { "VATTR_TEXCOORD_2", VATTR_TEXCOORD_2 },
          { "VATTR_TEXCOORD_3", VATTR_TEXCOORD_3 },
          { "VATTR_TEXCOORD_4", VATTR_TEXCOORD_4 },
          { "VATTR_TEXCOORD_5", VATTR_TEXCOORD_5 },
          { "VATTR_TEXCOORD_6", VATTR_TEXCOORD_6 },
          { "VATTR_TEXCOORD_7", VATTR_TEXCOORD_7 },
          { "VATTR_COLOR_0", VATTR_COLOR_0 },
          { "VATTR_COLOR_1", VATTR_COLOR_1 },
          { "VATTR_TANGENT", VATTR_TANGENT },
          { "VATTR_BINORMAL", VATTR_BINORMAL },
          { "VATTR_BLENDWEIGHT", VATTR_BLENDWEIGHT },
          { "VATTR_BLENDINDEX", VATTR_BLENDINDEX }

        };

        for (unsigned i = 0; i != countof(vattr); ++i)
            src_len += sprintf(src + src_len, "#define %s %i \n", vattr[i].name, vattr[i].value);
    }

    switch (targetApi)
    {
    case RHI_DX9:
    {
        strcat(src, _ShaderDefine_DX9);
    }
    break;

    case RHI_DX11:
    {
        strcat(src, _ShaderDefine_DX11);
    }
    break;

    case RHI_GLES2:
    {
        src_len += sprintf(src + src_len, "%s", _ShaderDefine_GLES2);
    }
    break;

    case RHI_METAL:
    {
        const char* s = srcText;
        const char* decl;
        bool vp_buf_declared[16];
        bool fp_buf_declared[16];
        bool fp_tex_declared[16];

        for (unsigned i = 0; i != countof(vp_buf_declared); ++i)
            vp_buf_declared[i] = false;
        for (unsigned i = 0; i != countof(fp_buf_declared); ++i)
            fp_buf_declared[i] = false;
        for (unsigned i = 0; i != countof(fp_tex_declared); ++i)
            fp_tex_declared[i] = false;

        while ((decl = strstr(s, "DECL_FPROG_BUFFER")))
        {
            int i = 0;

            sscanf(decl, "DECL_FPROG_BUFFER(%i,", &i);

            src_len += sprintf(src + src_len, "#define FPROG_IN_BUFFER_%i  ,constant __FP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, i);
            src_len += sprintf(src + src_len, "#define FPROG_BUFFER_%i    constant packed_float4* FP_Buffer%i = buf%i->data; \n", i, i, i);
            fp_buf_declared[i] = true;

            s += strlen("DECL_FPROG_BUFFER");
        }
        for (unsigned i = 0; i != countof(fp_buf_declared); ++i)
        {
            if (!fp_buf_declared[i])
            {
                src_len += sprintf(src + src_len, "#define FPROG_IN_BUFFER_%i \n", i);
                src_len += sprintf(src + src_len, "#define FPROG_BUFFER_%i \n", i);
            }
        }

        s = srcText;
        while ((decl = strstr(s, "DECL_FP_SAMPLER2D")))
        {
            int i = 0;

            sscanf(decl, "DECL_FP_SAMPLER2D(%i,", &i);

            src_len += sprintf(src + src_len, "#define FPROG_IN_TEXTURE_%i  , texture2d<float> fp_tex%i [[ texture(%i) ]]\n", i, i, i);
            src_len += sprintf(src + src_len, "#define FPROG_SAMPLER_%i   , sampler fp_tex%i_sampler [[ sampler(%i) ]]\n", i, i, i);
            fp_tex_declared[i] = true;

            s += strlen("DECL_FP_SAMPLER2D");
        }
        s = srcText;
        while ((decl = strstr(s, "DECL_FP_SAMPLERCUBE")))
        {
            int i = 0;

            sscanf(decl, "DECL_FP_SAMPLERCUBE(%i,", &i);

            src_len += sprintf(src + src_len, "#define FPROG_IN_TEXTURE_%i  , texturecube<float> fp_tex%i [[ texture(%i) ]]\n", i, i, i);
            src_len += sprintf(src + src_len, "#define FPROG_SAMPLER_%i   , sampler fp_tex%i_sampler [[ sampler(%i) ]]\n", i, i, i);
            fp_tex_declared[i] = true;

            s += strlen("DECL_FP_SAMPLER2D");
        }
        for (unsigned i = 0; i != countof(fp_tex_declared); ++i)
        {
            if (!fp_tex_declared[i])
            {
                src_len += sprintf(src + src_len, "#define FPROG_IN_TEXTURE_%i \n", i);
                src_len += sprintf(src + src_len, "#define FPROG_SAMPLER_%i \n", i);
            }
        }

        s = srcText;
        while ((decl = strstr(s, "DECL_VPROG_BUFFER")))
        {
            int i = 0;

            sscanf(decl, "DECL_VPROG_BUFFER(%i,", &i);

            src_len += sprintf(src + src_len, "#define VPROG_IN_BUFFER_%i  ,constant __VP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, 1 + i);
            src_len += sprintf(src + src_len, "#define VPROG_BUFFER_%i    constant packed_float4* VP_Buffer%i = buf%i->data; \n", i, i, i);
            vp_buf_declared[i] = true;

            s += strlen("DECL_VPROG_BUFFER");
        }
        for (unsigned i = 0; i != countof(vp_buf_declared); ++i)
        {
            if (!vp_buf_declared[i])
            {
                src_len += sprintf(src + src_len, "#define VPROG_IN_BUFFER_%i \n", i);
                src_len += sprintf(src + src_len, "#define VPROG_BUFFER_%i \n", i);
            }
        }

        strcat(src, _ShaderDefine_Metal);
    }
    break;
    }

    strcat(src, srcText);

    const char* argv[] =
    {
      "<mcpp>", // we just need first arg
      "-P", // do not output #line directives
      "-C", // keep comments
      MCPP_Text
    };

    //DAVA::Logger::Info( "src=\n%s\n", src );
    _PreprocessedText = preprocessedText;
    mcpp__set_input(src, static_cast<unsigned>(strlen(src)));

    mcpp_set_out_func(&_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf);
    mcpp_lib_main(countof(argv), (char**)argv);
    _PreprocessedText = 0;
    switch (targetApi)
    {
    case RHI_DX11:
        preprocessedText->insert(0, _ShaderHeader_DX11);
        break;

    case RHI_DX9:
        preprocessedText->insert(0, _ShaderHeader_DX9);
        break;

    case RHI_GLES2:
        preprocessedText->insert(0, _ShaderHeader_GLES2);
            #if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        preprocessedText->insert(0, "precision highp float;\n");
            #endif
        break;

    case RHI_METAL:
        preprocessedText->insert(0, _ShaderHeader_Metal);
        break;
    }

    //DAVA::Logger::Info( "pre-processed=\n%s\n", preprocessedText->c_str() );
}

//------------------------------------------------------------------------------

void UpdateProg(Api targetApi, ProgType progType, const DAVA::FastName& uid, const char* srcText)
{
    std::string txt;
    std::vector<uint8>* bin = nullptr;

    PreProcessSource(targetApi, srcText, &txt);

    for (unsigned i = 0; i != _ProgInfo.size(); ++i)
    {
        if (_ProgInfo[i].uid == uid)
        {
            bin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if (!bin)
    {
        _ProgInfo.push_back(ProgInfo());

        _ProgInfo.back().uid = uid;
        bin = &(_ProgInfo.back().bin);
    }

    bin->clear();
    bin->insert(bin->begin(), (const uint8*)(&(txt[0])), (const uint8*)(&(txt[txt.length() - 1]) + 1));
    bin->push_back(0);
}

//------------------------------------------------------------------------------

void UpdateProgBinary(Api targetApi, ProgType progType, const DAVA::FastName& uid, const void* bin, unsigned binSize)
{
    std::vector<uint8>* pbin = nullptr;

    for (unsigned i = 0; i != _ProgInfo.size(); ++i)
    {
        if (_ProgInfo[i].uid == uid)
        {
            pbin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if (!pbin)
    {
        _ProgInfo.push_back(ProgInfo());

        _ProgInfo.back().uid = uid;
        pbin = &(_ProgInfo.back().bin);
    }

    pbin->clear();
    pbin->insert(pbin->begin(), (const uint8*)(bin), (const uint8*)(bin) + binSize);
    pbin->push_back(0);
}

} // namespace ShaderCache
} // namespace rhi
