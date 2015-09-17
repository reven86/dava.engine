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


    #include "GameCore.h"
    
    #include "Render/RHI/rhi_Public.h"
    #include "Render/RHI/Common/rhi_Private.h"
    #include "Render/RHI/Common/dbg_StatSet.h"
    #include "Render/RHI/rhi_ShaderCache.h"
    #include "Render/RHI/rhi_ShaderSource.h"

    #include "Debug/Profiler.h"
#include "Render/RenderBase.h"

    #include "Render/RHI/dbg_Draw.h"

    #include "FileSystem/DynamicMemoryFile.h"


using namespace DAVA;

GameCore::GameCore()
  : inited(false)
{
}

GameCore::~GameCore()
{
}

void    
GameCore::SetupTriangle()
{
    triangle.vb     = rhi::HVertexBuffer(rhi::VertexBuffer::Create( 3*sizeof(VertexP) ));
    triangle.v_cnt  = 3;
    triangle.ib     = rhi::HIndexBuffer(rhi::IndexBuffer::Create( 3*sizeof(uint16) ));
    
    VertexP*    v = (VertexP*)rhi::VertexBuffer::Map( triangle.vb, 0, 3*sizeof(VertexP) );

    if( v )
    {
        v[0].x = -0.52f;
        v[0].y = -0.10f;
        v[0].z = 0.0f;
        
        v[1].x = 0.0f;
        v[1].y = 0.52f;
        v[1].z = 0.0f;
        
        v[2].x = 0.52f;
        v[2].y = -0.10f;
        v[2].z = 0.0f;

        rhi::VertexBuffer::Unmap( triangle.vb );
    }

    uint16  i[3] = { 0, 1, 2 };

    rhi::IndexBuffer::Update( triangle.ib, i, 0, 3*sizeof(uint16) );


    rhi::ShaderCache::UpdateProg
    ( 
        rhi::HostApi(), rhi::PROG_VERTEX, FastName("vp-simple"),
        "VPROG_IN_BEGIN\n"
        "    VPROG_IN_POSITION\n"
        "VPROG_IN_END\n"
        "\n"
        "VPROG_OUT_BEGIN\n"
        "    VPROG_OUT_POSITION\n"
        "VPROG_OUT_END\n"
        "\n"
        "VPROG_BEGIN\n"
        "\n"
        "    float3 in_pos = VP_IN_POSITION;"
        "    VP_OUT_POSITION = float4(in_pos.x,in_pos.y,in_pos.z,1.0);\n"
        "\n"
        "VPROG_END\n"
    );
    rhi::ShaderCache::UpdateProg
    ( 
        rhi::HostApi(), rhi::PROG_FRAGMENT, FastName("fp-simple"),
        "FPROG_IN_BEGIN\n"
        "FPROG_IN_END\n"
        "\n"
        "FPROG_OUT_BEGIN\n"
        "    FPROG_OUT_COLOR\n"
        "FPROG_OUT_END\n"
        "\n"
        "DECL_FPROG_BUFFER(0,4)\n"
        "\n"
        "FPROG_BEGIN\n"
        "    FP_OUT_COLOR = float4(FP_Buffer0[0]);\n"
        "FPROG_END\n"
    );


    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement( rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3 );
    psDesc.vprogUid = FastName("vp-simple");
    psDesc.fprogUid = FastName("fp-simple");

    triangle.ps       = rhi::HPipelineState(rhi::PipelineState::Create( psDesc ));
    triangle.fp_const = rhi::HConstBuffer(rhi::PipelineState::CreateFragmentConstBuffer( triangle.ps, 0 ));
}


void
GameCore::SetupCube()
{
//    cube.vb = rhi::VertexBuffer::Create( 3*2*6*sizeof(VertexPNT_ex) );
    cube.vb = rhi::HVertexBuffer(rhi::VertexBuffer::Create( 3*2*6*sizeof(VertexPNT) ));
//-    cube.ib = rhi::InvalidHandle;

    float       sz    = 0.2f;
    float       u0    = 0.0f;
    float       u1    = 1.0f;
    float       v0    = 0.0f;
    float       v1    = 1.0f;

    VertexPNT   v[36] = 
    {
        { -sz,-sz,-sz, 0,0,-1, u0,v1 }, { -sz,sz,-sz, 0,0,-1, u0,v0 }, { sz,-sz,-sz, 0,0,-1, u1,v1 },
        { -sz,sz,-sz, 0,0,-1, u0,v0 }, { sz,sz,-sz, 0,0,-1, u1,v0 }, { sz,-sz,-sz, 0,0,-1, u1,v1 },

        { sz,-sz,-sz, 1,0,0, u0,v1 }, { sz,sz,-sz, 1,0,0, u0,v0 }, { sz,-sz,sz, 1,0,0, u1,v1 },
        { sz,sz,-sz, 1,0,0, u0,v0 }, { sz,sz,sz, 1,0,0, u1,v0 }, { sz,-sz,sz, 1,0,0, u1,v1 },
    
        { sz,-sz,sz, 0,0,1, u0,v1 }, { sz,sz,sz, 0,0,1, u0,v0 }, { -sz,-sz,sz, 0,0,1, u1,v1 },    
        { sz,sz,sz, 0,0,1, u0,v0 }, { -sz,sz,sz, 0,0,1, u1,v0 }, { -sz,-sz,sz, 0,0,1, u1,v1 },
    
        { -sz,-sz,sz, -1,0,0, u0,v1 }, { -sz,sz,sz, -1,0,0, u0,v0 }, { -sz,sz,-sz, -1,0,0, u1,v0 },
        { -sz,sz,-sz, -1,0,0, u1,v0 }, { -sz,-sz,-sz, -1,0,0, u1,v1 }, { -sz,-sz,sz, -1,0,0, u0,v1 },

        { -sz,sz,-sz, 0,1,0, u0,v1 }, { -sz,sz,sz, 0,1,0, u0,v0 }, { sz,sz,-sz, 0,1,0, u1,v1 },
        { -sz,sz,sz, 0,1,0, u0,v0 }, { sz,sz,sz, 0,1,0, u1,v0 }, { sz,sz,-sz, 0,1,0, u1,v1 },
                
        { -sz,-sz,-sz, 0,-1,0, u0,v0 }, { sz,-sz,-sz, 0,-1,0, u1,v0 }, { -sz,-sz,sz, 0,-1,0, u0,v1 },
        { sz,-sz,-sz, 0,-1,0, u1,v0 }, { sz,-sz,sz, 0,-1,0, u1,v1 }, { -sz,-sz,sz, 0,-1,0, u0,v1 }
    };
    
/*
    VertexPNT_ex    v[36] = 
    {
        { -sz,-sz,-sz,0, 0,0,-1, u0,v1 }, { -sz,sz,-sz,0, 0,0,-1, u0,v0 }, { sz,-sz,-sz,0, 0,0,-1, u1,v1 },
        { -sz,sz,-sz,0, 0,0,-1, u0,v0 }, { sz,sz,-sz,0, 0,0,-1, u1,v0 }, { sz,-sz,-sz,0, 0,0,-1, u1,v1 },

        { sz,-sz,-sz,0, 1,0,0, u0,v1 }, { sz,sz,-sz,0, 1,0,0, u0,v0 }, { sz,-sz,sz,0, 1,0,0, u1,v1 },
        { sz,sz,-sz,0, 1,0,0, u0,v0 }, { sz,sz,sz,0, 1,0,0, u1,v0 }, { sz,-sz,sz,0, 1,0,0, u1,v1 },
    
        { sz,-sz,sz,0, 0,0,1, u0,v1 }, { sz,sz,sz,0, 0,0,1, u0,v0 }, { -sz,-sz,sz,0, 0,0,1, u1,v1 },    
        { sz,sz,sz,0, 0,0,1, u0,v0 }, { -sz,sz,sz,0, 0,0,1, u1,v0 }, { -sz,-sz,sz,0, 0,0,1, u1,v1 },
    
        { -sz,-sz,sz,0, -1,0,0, u0,v1 }, { -sz,sz,sz,0, -1,0,0, u0,v0 }, { -sz,sz,-sz,0, -1,0,0, u1,v0 },
        { -sz,sz,-sz,0, -1,0,0, u1,v0 }, { -sz,-sz,-sz,0, -1,0,0, u1,v1 }, { -sz,-sz,sz,0, -1,0,0, u0,v1 },

        { -sz,sz,-sz,0, 0,1,0, u0,v1 }, { -sz,sz,sz,0, 0,1,0, u0,v0 }, { sz,sz,-sz,0, 0,1,0, u1,v1 },
        { -sz,sz,sz,0, 0,1,0, u0,v0 }, { sz,sz,sz,0, 0,1,0, u1,v0 }, { sz,sz,-sz,0, 0,1,0, u1,v1 },
                
        { -sz,-sz,-sz,0, 0,-1,0, u0,v0 }, { sz,-sz,-sz,0, 0,-1,0, u1,v0 }, { -sz,-sz,sz,0, 0,-1,0, u0,v1 },
        { sz,-sz,-sz,0, 0,-1,0, u1,v0 }, { sz,-sz,sz,0, 0,-1,0, u1,v1 }, { -sz,-sz,sz,0, 0,-1,0, u0,v1 }
    };
*/
/*
    VertexPNT_ex    v[36] = 
    {
        { -sz,-sz,-sz,0, u0,v1, 0,0,-1 }, { -sz,sz,-sz,0, u0,v0, 0,0,-1 }, { sz,-sz,-sz,0, u1,v1, 0,0,-1 },
        { -sz,sz,-sz,0, u0,v0, 0,0,-1 }, { sz,sz,-sz,0, u1,v0, 0,0,-1 }, { sz,-sz,-sz,0, u1,v1, 0,0,-1 },

        { sz,-sz,-sz,0, u0,v1, 1,0,0 }, { sz,sz,-sz,0, u0,v0, 1,0,0 }, { sz,-sz,sz,0, u1,v1, 1,0,0 },
        { sz,sz,-sz,0, u0,v0, 1,0,0 }, { sz,sz,sz,0, u1,v0, 1,0,0 }, { sz,-sz,sz,0, u1,v1, 1,0,0 },
    
        { sz,-sz,sz,0, u0,v1, 0,0,1 }, { sz,sz,sz,0, u0,v0, 0,0,1 }, { -sz,-sz,sz,0, u1,v1, 0,0,1 },    
        { sz,sz,sz,0, u0,v0, 0,0,1 }, { -sz,sz,sz,0, u1,v0, 0,0,1 }, { -sz,-sz,sz,0, u1,v1, 0,0,1 },
    
        { -sz,-sz,sz,0, u0,v1, -1,0,0 }, { -sz,sz,sz,0, u0,v0, -1,0,0 }, { -sz,sz,-sz,0, u1,v0, -1,0,0 },
        { -sz,sz,-sz,0, u1,v0, -1,0,0 }, { -sz,-sz,-sz,0, u1,v1, -1,0,0 }, { -sz,-sz,sz,0, u0,v1, -1,0,0 },

        { -sz,sz,-sz,0, u0,v1, 0,1,0 }, { -sz,sz,sz,0, u0,v0, 0,1,0 }, { sz,sz,-sz,0, u1,v1, 0,1,0 },
        { -sz,sz,sz,0, u0,v0, 0,1,0 }, { sz,sz,sz,0, u1,v0, 0,1,0 }, { sz,sz,-sz,0, u1,v1, 0,1,0 },
                
        { -sz,-sz,-sz,0, u0,v0, 0,-1,0 }, { sz,-sz,-sz,0, u1,v0, 0,-1,0 }, { -sz,-sz,sz,0, u0,v1, 0,-1,0 },
        { sz,-sz,-sz,0, u1,v0, 0,-1,0 }, { sz,-sz,sz,0, u1,v1, 0,-1,0 }, { -sz,-sz,sz,0, u0,v1, 0,-1,0 }
    };
*/
    rhi::VertexBuffer::Update( cube.vb, v, 0, sizeof(v) );

    rhi::Texture::Descriptor    tdesc(128,128,rhi::TEXTURE_FORMAT_R8G8B8A8);
    
//    tdesc.autoGenMipmaps = true;
    cube.tex = rhi::HTexture(rhi::Texture::Create( tdesc ));
    
    uint8*  tex = (uint8*)(rhi::Texture::Map( cube.tex ));

    if( tex )
    {
        uint8   color1[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        uint8   color2[4] = { 0x80, 0x80, 0x80, 0xFF };
//        uint8   color1[4] = { 0xFF, 0x00, 0x00, 0xFF };
//        uint8   color2[4] = { 0x80, 0x00, 0x00, 0xFF };
        uint32  cell_size = 8;

        for( unsigned y=0; y!=128; ++y )
        {
            for( unsigned x=0; x!=128; ++x )
            {
                uint8*  p = tex + y*sizeof(uint32)*128 + x*sizeof(uint32);
                uint8*  c = ( ((y/cell_size) & 0x1) ^ ((x/cell_size) & 0x1) )
                            ? color1
                            : color2;

                memcpy( p, c, sizeof(uint32) );                
            }
        }

        rhi::Texture::Unmap( cube.tex );
    }

    rhi::SamplerState::Descriptor   sdesc;
    
    sdesc.fragmentSamplerCount          = 1;
    sdesc.fragmentSampler[0].addrU      = rhi::TEXADDR_WRAP;
    sdesc.fragmentSampler[0].addrV      = rhi::TEXADDR_WRAP;
    sdesc.fragmentSampler[0].minFilter  = rhi::TEXFILTER_LINEAR;
    sdesc.fragmentSampler[0].magFilter  = rhi::TEXFILTER_LINEAR;
    sdesc.fragmentSampler[0].mipFilter  = rhi::TEXMIPFILTER_NONE;
    
    cube.samplerState = rhi::HSamplerState(rhi::SamplerState::Create( sdesc ));


    rhi::ShaderCache::UpdateProg
    ( 
        rhi::HostApi(), rhi::PROG_VERTEX, FastName("vp-shaded"),
        "VPROG_IN_BEGIN\n"
        "    VPROG_IN_POSITION\n"
        "    VPROG_IN_NORMAL\n"
        "    VPROG_IN_TEXCOORD\n"
        "VPROG_IN_END\n"
        "\n"
        "VPROG_OUT_BEGIN\n"
        "    VPROG_OUT_POSITION\n"
        "    VPROG_OUT_TEXCOORD0(uv,2)\n"
        "    VPROG_OUT_TEXCOORD1(color,4)\n"
        "VPROG_OUT_END\n"
        "\n"
        "DECL_VPROG_BUFFER(0,16)\n"
        "DECL_VPROG_BUFFER(1,16)\n"
        "\n"
        "VPROG_BEGIN\n"
        "\n"
        "    float3 in_pos      = VP_IN_POSITION;\n"
        "    float3 in_normal   = VP_IN_NORMAL;\n"
        "    float2 in_texcoord = VP_IN_TEXCOORD;\n"
        "    float4x4 ViewProjection = float4x4( VP_Buffer0[0], VP_Buffer0[1], VP_Buffer0[2], VP_Buffer0[3] );\n"
        "    float4x4 World = float4x4( VP_Buffer1[0], VP_Buffer1[1], VP_Buffer1[2], VP_Buffer1[3] );\n"
//        "    float3x3 World3 = float3x3( (float3)(float4(VP_Buffer1[0])), (float3)(float4(VP_Buffer1[1])), (float3)(float4(VP_Buffer1[2])) );\n"
//        "    float3x3 World3 = float3x3( float3(VP_Buffer1[0]), float3(VP_Buffer1[1]), float3(VP_Buffer1[2]) );\n"
        "    float3x3 World3 = VP_BUF_FLOAT3X3(1,0);"
        "    float4 wpos = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), World );\n"
        "    float i   = dot( float3(0,0,-1), normalize(mul(float3(in_normal),World3)) );\n"
        "    VP_OUT_POSITION   = mul( wpos, ViewProjection );\n"
        "    VP_OUT(uv)        = in_texcoord;\n"
        "    VP_OUT(color)     = float4(i,i,i,1.0);\n"
        "\n"
        "VPROG_END\n"
/*
"struct VP_Input"
"{\n" 
"    packed_float3 position;\n"
"    packed_float3 normal;\n"
"    packed_float2 texcoord;\n"
"};\n"
"struct VP_Output\n" 
"{\n"
"    float4 position [[ position ]];\n" 
"    float4 color [[ user(texturecoord) ]];\n" 
"};\n"
"struct VP_Buffer0 { packed_float4 data[16]; };\n"
"struct VP_Buffer1 { packed_float4 data[16]; };\n"
"vertex VP_Output vp_main\n"
"( \n"
"    constant VP_Input*   in    [[ buffer(0) ]],\n"
"    constant VP_Buffer0* buf0  [[ buffer(1) ]],\n"
"    constant VP_Buffer1* buf1  [[ buffer(2) ]],\n"
"    uint                 vid   [[ vertex_id ]]\n"
")\n"
"{\n"
"    VP_Output   OUT;\n"
"    VP_Input    IN  = in[vid];\n"
"\n"
"    float4x4 ViewProjection = float4x4( buf0->data[0], buf0->data[1], buf0->data[2], buf0->data[3] );\n"
"    float4x4 World = float4x4( buf1->data[0], buf1->data[1], buf1->data[2], buf1->data[3] );\n"
"    float3x3 World3 = float3x3( float3(float4(buf1->data[0])), float3(float4(buf1->data[1])), float3(float4(buf1->data[2])) );\n"
"    float4 wpos = World * float4(IN.position[0],IN.position[1],IN.position[2],1.0);\n"
"    float i   = dot( float3(0,0,-1), normalize(World3*float3(IN.normal)) );\n"
"    OUT.position   = ViewProjection * wpos;\n"
"    OUT.color      = float4(i,i,i,1.0);\n"
"\n"
"    return OUT;\n"
"}\n"
*/
/*
"precision highp float;\n"
#if DV_USE_UNIFORMBUFFER_OBJECT
        "uniform VP_Buffer0_Block { vec4 VP_Buffer0[16]; };\n"
        "uniform VP_Buffer1_Block { vec4 VP_Buffer1[16]; };\n"
#else
        "uniform vec4 VP_Buffer0[16];\n"
        "uniform vec4 VP_Buffer1[16];\n"
#endif        
        "attribute vec4 attr_position;\n"
        "attribute vec3 attr_normal;\n"
        "attribute vec2 attr_texcoord;\n"
        "varying vec3 var_Color;\n"
        "void main()\n"
        "{\n"
        "    mat4 ViewProjection = mat4( VP_Buffer0[0], VP_Buffer0[1], VP_Buffer0[2], VP_Buffer0[3] );\n"
        "    mat4 World = mat4( VP_Buffer1[0], VP_Buffer1[1], VP_Buffer1[2], VP_Buffer1[3] );\n"
        "    vec4 wpos = World * vec4(attr_position.x,attr_position.y,attr_position.z,1.0);\n"
//        "    float i   = dot( vec3(0,0,-1), normalize(mat3(World)*attr_normal) );\n"
        "    float i   = dot( vec3(0,0,-1), normalize( mat3(vec3(World[0].x,World[0].y,World[0].z),vec3(World[1].x,World[1].y,World[1].z),vec3(World[2].x,World[2].y,World[2].z)) * attr_normal) );\n"
        "    gl_Position   = ViewProjection * wpos;\n"
//        "    var_Color.rgb = i;\n"
        "    var_Color.rgb = vec3(i,i,i);\n"
        "}\n"
*/    
    );
    rhi::ShaderCache::UpdateProg
    (
        rhi::HostApi(), rhi::PROG_FRAGMENT, FastName("fp-shaded"),
        "FPROG_IN_BEGIN\n"
        "FPROG_IN_TEXCOORD0(uv,2)\n"
        "FPROG_IN_TEXCOORD1(color,4)\n"
        "FPROG_IN_END\n"
        "\n"
        "FPROG_OUT_BEGIN\n"
        "    FPROG_OUT_COLOR\n"
        "FPROG_OUT_END\n"
        "\n"
        "DECL_FP_SAMPLER2D(0)\n"
        "\n"
        "\n"
        "DECL_FPROG_BUFFER(0,4)\n"
        "\n"
        "FPROG_BEGIN\n"
        "    float4  diffuse = FP_TEXTURE2D( 0, FP_IN(uv) );\n"
        "    FP_OUT_COLOR = diffuse * float4(FP_Buffer0[0]) * FP_IN(color);\n"
        "FPROG_END\n"
/*
"struct FP_Input\n"
"{\n"
"    float4 position [[position]];\n" 
"    float4 color [[user(texturecoord)]];\n"
"};\n"
"struct FP_Buffer0 { packed_float4 data[4]; };\n"
"float4 fragment fp_main\n"
"(\n"
"    FP_Input IN                [[ stage_in ]],\n"
"    constant FP_Buffer0* buf0  [[ buffer(0) ]]\n"
")\n"
"{\n"
"    float4 clr = float4(buf0->data[0]) * IN.color;\n"
"    return clr;\n"
"}\n"
*/
/*
"precision highp float;\n"
#if DV_USE_UNIFORMBUFFER_OBJECT
        "uniform FP_Buffer0_Block { vec4 FP_Buffer0[4]; };\n"
#else
        "uniform vec4 FP_Buffer0[4];\n"
#endif
        "varying vec3 var_Color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor.rgb = FP_Buffer0[0].xyz * var_Color;\n"
        "    gl_FragColor.a   = FP_Buffer0[0].a;\n"
        "}\n"
*/    
    );


    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement( rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3 );
    psDesc.vertexLayout.AddElement( rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3 );
    psDesc.vertexLayout.AddElement( rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2 );
    psDesc.vprogUid = FastName("vp-shaded");
    psDesc.fprogUid = FastName("fp-shaded");

    cube.ps          = rhi::HPipelineState(rhi::PipelineState::Create( psDesc ));
    cube.vp_const[0] = rhi::HConstBuffer(rhi::PipelineState::CreateVertexConstBuffer( cube.ps, 0 ));
    cube.vp_const[1] = rhi::HConstBuffer(rhi::PipelineState::CreateVertexConstBuffer( cube.ps, 1 ));
    cube.fp_const    = rhi::HConstBuffer(rhi::PipelineState::CreateFragmentConstBuffer( cube.ps, 0 ));
    
    
    rhi::VertexLayout    vb_layout;
    vb_layout.Clear();
    vb_layout.AddElement( rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3 );
    vb_layout.AddElement( rhi::VS_PAD, 0, rhi::VDT_UINT8, 4 );
    vb_layout.AddElement( rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2 );
    vb_layout.AddElement( rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3 );
    cube.vb_layout = rhi::VertexLayout::UniqueId( vb_layout );
    cube.vb_layout = rhi::VertexLayout::InvalidUID;

    rhi::TextureSetDescriptor   td;
    td.fragmentTextureCount = 1;
    td.fragmentTexture[0]   = cube.tex;
    cube.texSet = rhi::AcquireTextureSet( td );

    cube_t0     = SystemTimer::Instance()->AbsoluteMS();
    cube_angle  = 0;
}

void
GameCore::SetupRT()
{
    rtQuad.vb = rhi::HVertexBuffer(rhi::VertexBuffer::Create( 3*2*sizeof(VertexPT) ));
//-    rtQuad.ib = rhi::InvalidHandle;
    
    const VertexPT  v[2*3] = 
    { 
        {-1,1,0, 0,1}, {1,1,0, 1,1}, {1,-1,0, 1,0},
        {-1,1,0, 0,1}, {1,-1,0, 1,0}, {-1,-1,0, 0,0}
    };

    rhi::VertexBuffer::Update( rtQuad.vb, v, 0, sizeof(v) );


    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement( rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3 );
    psDesc.vertexLayout.AddElement( rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2 );
    psDesc.vprogUid = FastName("vp-copy");
    psDesc.fprogUid = FastName("fp-copy");

    rhi::ShaderCache::UpdateProg
    ( 
        rhi::HostApi(), rhi::PROG_VERTEX, FastName("vp-copy"),
        "VPROG_IN_BEGIN\n"
        "    VPROG_IN_POSITION\n"
        "    VPROG_IN_TEXCOORD\n"
        "VPROG_IN_END\n"
        "\n"
        "VPROG_OUT_BEGIN\n"
        "    VPROG_OUT_POSITION\n"
        "    VPROG_OUT_TEXCOORD0(uv,2)\n"
        "VPROG_OUT_END\n"
        "\n"
        "DECL_VPROG_BUFFER(0,16)\n"
        "DECL_VPROG_BUFFER(1,16)\n"
        "\n"
        "VPROG_BEGIN\n"
        "\n"
        "    float3 in_pos      = VP_IN_POSITION;\n"
        "    float2 in_texcoord = VP_IN_TEXCOORD;\n"
        "    float4x4 ViewProjection = float4x4( VP_Buffer0[0], VP_Buffer0[1], VP_Buffer0[2], VP_Buffer0[3] );\n"
        "    float4x4 World = float4x4( VP_Buffer1[0], VP_Buffer1[1], VP_Buffer1[2], VP_Buffer1[3] );\n"
        "    float4 wpos = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), World );\n"
        "    VP_OUT_POSITION   = mul( wpos, ViewProjection );\n"
        "    VP_OUT(uv)        = in_texcoord;\n"
        "\n"
        "VPROG_END\n"
    );
    rhi::ShaderCache::UpdateProg
    (
        rhi::HostApi(), rhi::PROG_FRAGMENT, FastName("fp-copy"),
        "FPROG_IN_BEGIN\n"
        "FPROG_IN_TEXCOORD0(uv,2)\n"
        "FPROG_IN_END\n"
        "\n"
        "FPROG_OUT_BEGIN\n"
        "    FPROG_OUT_COLOR\n"
        "FPROG_OUT_END\n"
        "\n"
        "DECL_FP_SAMPLER2D(0)\n"
        "\n"
        "\n"
        "FPROG_BEGIN\n"
        "    FP_OUT_COLOR = FP_TEXTURE2D( 0, FP_IN(uv) );\n"
        "FPROG_END\n"
    );

    rtQuad.ps          = rhi::HPipelineState(rhi::PipelineState::Create( psDesc ));
    rtQuad.vp_const[0] = rhi::HConstBuffer(rhi::PipelineState::CreateVertexConstBuffer( rtQuad.ps, 0 ));
    rtQuad.vp_const[1] = rhi::HConstBuffer(rhi::PipelineState::CreateVertexConstBuffer( rtQuad.ps, 1 ));


    rhi::SamplerState::Descriptor   sdesc;
    
    sdesc.fragmentSamplerCount          = 1;
    sdesc.fragmentSampler[0].addrU      = rhi::TEXADDR_CLAMP;
    sdesc.fragmentSampler[0].addrV      = rhi::TEXADDR_CLAMP;
    sdesc.fragmentSampler[0].minFilter  = rhi::TEXFILTER_LINEAR;
    sdesc.fragmentSampler[0].magFilter  = rhi::TEXFILTER_LINEAR;
    sdesc.fragmentSampler[0].mipFilter  = rhi::TEXMIPFILTER_NONE;
    
    rtQuad.samplerState = rhi::HSamplerState(rhi::SamplerState::Create( sdesc ));


    rhi::Texture::Descriptor    colorDesc(512,512,rhi::TEXTURE_FORMAT_R8G8B8A8);
    rhi::Texture::Descriptor    depthDesc(512,512,rhi::TEXTURE_FORMAT_D24S8);
    
    colorDesc.isRenderTarget = true;
    
    rtColor         = rhi::Texture::Create( colorDesc );
    rtDepthStencil  = rhi::Texture::Create( depthDesc );


    rhi::TextureSetDescriptor   tsDesc;
    
    tsDesc.fragmentTextureCount = 1;
    tsDesc.fragmentTexture[0]   = rhi::HTexture(rtColor);

    rtQuadBatch.vertexStreamCount   = 1;
    rtQuadBatch.vertexStream[0]     = rtQuad.vb;
//-    rtQuadBatch.indexBuffer         = rhi::InvalidHandle;
    rtQuadBatch.vertexConstCount    = 2;
    rtQuadBatch.vertexConst[0]      = rtQuad.vp_const[0];
    rtQuadBatch.vertexConst[1]      = rtQuad.vp_const[1];
    rtQuadBatch.fragmentConstCount  = 0;
    rtQuadBatch.renderPipelineState = rtQuad.ps;
    rtQuadBatch.samplerState        = rtQuad.samplerState;
    rtQuadBatch.primitiveType       = rhi::PRIMITIVE_TRIANGLELIST;
    rtQuadBatch.primitiveCount      = 2;
    rtQuadBatch.textureSet          = rhi::AcquireTextureSet( tsDesc );
}


void GameCore::SetupTank()
{
    SceneFileV2 *sceneFile = new SceneFileV2();
    sceneFile->EnableDebugLog(false);
    SceneArchive *archive = sceneFile->LoadSceneArchive("~res:/3d/test.sc2");
    if (!archive) return;
    for (int32 i = 0, sz = archive->dataNodes.size(); i < sz; ++i)
    {
        String name = archive->dataNodes[i]->GetString("##name");
        if (name == "PolygonGroup")
        {            
            KeyedArchive *keyedArchive = archive->dataNodes[i];
            int32 vertexFormat = keyedArchive->GetInt32("vertexFormat");
            int32 vertexStride = GetVertexSize(vertexFormat);
            int32 vertexCount = keyedArchive->GetInt32("vertexCount");
            int32 indexCount = keyedArchive->GetInt32("indexCount");
            int32 textureCoordCount = keyedArchive->GetInt32("textureCoordCount");            
            int32 cubeTextureCoordCount = keyedArchive->GetInt32("cubeTextureCoordCount");
            if (vertexFormat != (EVF_VERTEX | EVF_NORMAL | EVF_TEXCOORD0))
                continue; //for now only this format
            int32 formatPacking = keyedArchive->GetInt32("packing");

            {
                int size = keyedArchive->GetByteArraySize("vertices");
                if (size != vertexCount * vertexStride)
                {
                    Logger::Error("PolygonGroup::Load - Something is going wrong, size of vertex array is incorrect");
                    return;
                }

                const uint8 * archiveData = keyedArchive->GetByteArray("vertices");
                rhi::Handle vb = rhi::VertexBuffer::Create(vertexCount * vertexStride);
                rhi::VertexBuffer::Update(vb, archiveData, 0, vertexCount * vertexStride);
                tank.vb.push_back(vb);
                /*uint8 *meshData = new uint8[vertexCount * vertexStride];
                Memcpy(meshData, archiveData, size); //all streams in data required - just copy*/
            }

            int32 indexFormat = keyedArchive->GetInt32("indexFormat");
            {
                int size = keyedArchive->GetByteArraySize("indices");
                uint16 *indexArray = new uint16[indexCount];
                const uint8 * archiveData = keyedArchive->GetByteArray("indices");
                rhi::Handle ib = rhi::IndexBuffer::Create( rhi::IndexBuffer::Descriptor(indexCount*INDEX_FORMAT_SIZE[indexFormat]) );
                rhi::IndexBuffer::Update(ib, archiveData, 0, indexCount * INDEX_FORMAT_SIZE[indexFormat]);
                tank.ib.push_back(ib);
                tank.indCount.push_back(indexCount);
                /*memcpy(indexArray, archiveData, indexCount * INDEX_FORMAT_SIZE[indexFormat]);*/
            }
        }
    }

    Vector<Image* > images;

    ImageSystem::Instance()->Load("~res:/3d/test.png", images, 0);
    if (images.size())
    {
        Image *img = images[0];
        PixelFormat format = img->GetPixelFormat();
        uint32 w = img->GetWidth();
        uint32 h = img->GetHeight();
        tank.tex = rhi::Texture::Create( rhi::Texture::Descriptor(w,h,rhi::TEXTURE_FORMAT_R8G8B8A8) );
        uint8*  tex = (uint8*)(rhi::Texture::Map(tank.tex));
        memcpy(tex, img->GetData(), w*h*4);
        rhi::Texture::Unmap(tank.tex);
    }

    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    psDesc.vertexLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
    psDesc.vertexLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    psDesc.vprogUid = FastName("vp-shaded");
    psDesc.fprogUid = FastName("fp-shaded");

    tank.ps = rhi::PipelineState::Create(psDesc);
    tank.vp_const[0] = rhi::PipelineState::CreateVertexConstBuffer(tank.ps, 0);
    tank.vp_const[1] = rhi::PipelineState::CreateVertexConstBuffer(tank.ps, 1);
    tank.fp_const = rhi::PipelineState::CreateFragmentConstBuffer(tank.ps, 0);
    

      

    /*rhi::Handle ps;
    rhi::Handle vp_const[2];
    rhi::Handle fp_const;
    rhi::Handle tex;*/
}

void GameCore::OnAppStarted()
{

    struct
    {
        const char* file;
        const char* flag[16];
    } src[]
    {
        { "../../Tools/ResourceEditor/Data/Materials/Shaders/Default/materials-vp.cg", {"VERTEX_LIT",nullptr} }/*,
        { "../../Tools/ResourceEditor/Data/Materials/Shaders/Default/materials-vp.cg", {"PIXEL_LIT",nullptr} },
        { "../../Tools/ResourceEditor/Data/Materials/Shaders/Default/materials-vp.cg", {"SKINNING","PIXEL_LIT",nullptr} }
*/
    };

    
//    profiler::Start();    
    
    for( unsigned i=0; i!=countof(src); ++i )
    {
        File*   file = File::CreateFromSystemPath( src[i].file, File::OPEN|File::READ );
    
        if( file )
        {
            rhi::ShaderSource   vp;
            uint32              sz = file->GetSize();
            char                buf[64*1024];

            DVASSERT(sz < sizeof(buf));
            file->Read( buf, sz );
            buf[sz] = '\0';


            std::vector<std::string>    defines;
            
            for( unsigned k=0; k!=countof(src[i].flag); ++k )    
            {
                if( src[i].flag[k] )
                {
                    defines.push_back( src[i].flag[k] );
                    defines.push_back( "1" );
                }
                else
                {
                    break;
                }
            }
            if( vp.Construct( rhi::PROG_VERTEX, buf, defines ) )
            {
uint8       data[128*1024];
DAVA::File* f = DAVA::DynamicMemoryFile::Create( data, countof(data), DAVA::File::READ|DAVA::File::WRITE );

f->Seek(0,DAVA::File::SEEK_FROM_START);
vp.Save( f );

f->Seek(0,DAVA::File::SEEK_FROM_START);
vp.Load( f );

vp.Dump();

                //vp.Dump();
            }
        }
    }
    
//    profiler::Stop();
//    profiler::Dump();

/*
{
    File*   file = File::CreateFromSystemPath( "../../Tools/ResourceEditor/Data/Materials/Shaders/Default/materials-vp.cg", File::OPEN|File::READ );
    
    if( file )
    {
        rhi::ShaderSource   vp;
        uint32              sz = file->GetSize();
        char                buf[64*1024];

        DVASSERT(sz < sizeof(buf));
        file->Read( buf, sz );
        buf[sz] = '\0';


        std::vector<std::string>    defines;
        
        defines.push_back( "VERTEX_LIT" );
        defines.push_back( "1" );
        if( vp.Construct( rhi::PROG_VERTEX, buf, defines ) )
        {
            vp.Dump();
        }
    }
}
{
    File*   file = File::CreateFromSystemPath( "../../Tools/ResourceEditor/Data/Materials/Shaders/Default/materials-fp.cg", File::OPEN|File::READ );
    
    if( file )
    {
        rhi::ShaderSource   fp;
        uint32              sz = file->GetSize();
        char                buf[64*1024];

        DVASSERT(sz < sizeof(buf));
        file->Read( buf, sz );
        buf[sz] = '\0';


        std::vector<std::string>    defines;
        
        defines.push_back( "VERTEX_LIT" );
        defines.push_back( "1" );
        if( fp.Construct( rhi::PROG_FRAGMENT, buf, defines ) )
        {
            fp.Dump();
        }
    }
}
*/


    DbgDraw::EnsureInited();
    
    #if defined(__DAVAENGINE_WIN32__)
    {
    KeyedArchive*   opt = new KeyedArchive();
    char            title[128] = "RHI Cube  -  ";

    switch( rhi::HostApi() )
    {
        case rhi::RHI_DX9   : strcat( title, "DX9" ); break;
        case rhi::RHI_DX11  : strcat( title, "DX11" ); break;
        case rhi::RHI_GLES2 : strcat( title, "GL" ); break;
    }

    opt->SetInt32( "fullscreen", 0 );
    opt->SetInt32( "bpp", 32 );
    opt->SetString( String("title"), String(title) );

    DAVA::Core::Instance()->SetOptions(opt);
    }
    #endif


//    SetupTriangle();
    SetupCube();
//    SetupTank();
    SetupRT();

//    sceneRenderTest.reset(new SceneRenderTestV3());    

/*
    // ShaderSource smoke-test
    const char*  fp_src =
    "FPROG_IN_BEGIN\n"
    "FPROG_IN_TEXCOORD0(uv,2)\n"
    "FPROG_IN_TEXCOORD1(color,4)\n"
    "FPROG_IN_END\n"
    "\n"
    "FPROG_OUT_BEGIN\n"
    "    FPROG_OUT_COLOR\n"
    "FPROG_OUT_END\n"
    "\n"
    "property float4 tfactor : unique,dynamic :   def_value=1,1,1,1 ;\n"
    "property float4 bla[3]  : unique,dynamic :    ;\n"
    "property float2 scale   : unique,dynamic :    ;\n"
    "property float  aa      : unique,dynamic :    ;\n"
    "property float  bb      : unique,dynamic :    ;\n"
    "\n"
    "DECL_FP_SAMPLER2D(albedo)\n"
    "DECL_FP_SAMPLER2D(albedo2)\n"
    "\n"
    "\n"
    "FPROG_BEGIN\n"
    "    float4  diffuse = FP_TEXTURE2D( albedo, FP_IN(uv) );\n"
    "    float4  diffuse2 = FP_TEXTURE2D( albedo2, FP_IN(uv) );\n"
    "    FP_OUT_COLOR = tfactor * FP_IN(color);\n"
    "FPROG_END\n"
    "blending : src=src_alpha dst=inv_src_alpha\n"
    ;

    rhi::ShaderSource   fp;

    fp.Construct( rhi::PROG_FRAGMENT, fp_src );
    Logger::Info( "\n\n====================" );
    fp.Dump();
*/
/*
    // ShaderSource smoke-test
    const char*  vp_src =
    "VPROG_IN_BEGIN\n"
    "    VPROG_IN_POSITION\n"
    "    VPROG_IN_NORMAL\n"
    "    VPROG_IN_TEXCOORD\n"
    "VPROG_IN_END\n"
    "\n"
    "VPROG_OUT_BEGIN\n"
    "    VPROG_OUT_POSITION\n"
    "    VPROG_OUT_TEXCOORD0(uv,2)\n"
    "    VPROG_OUT_TEXCOORD1(color,4)\n"
    "VPROG_OUT_END\n"
    "\n"
//    "property float4x4 ViewProjection : shared,dynamic : ui_hidden=yes ;\n"
//    "property float4x4 World : unique,dynamic : ui_hidden=yes ;\n"
"property float4x4 worldViewProjMatrix : unique,dynamic : ui_hidden=yes ;\n"
"property float2 uvOffset : unique,static : ui_hidden=yes ;\n"
"property float2 uvScale : unique,static : ui_hidden=yes ;\n"
    "DECL_FP_SAMPLER2D(stuff)\n"
    ""
    "VPROG_BEGIN\n"
    "\n"
    "    float3 in_pos      = VP_IN_POSITION;\n"
    "    float3 in_normal   = VP_IN_NORMAL;\n"
    "    float2 in_texcoord = VP_IN_TEXCOORD;\n"
    "    float3x3 World3 = VP_BUF_FLOAT3X3(1,0);"
    "    float4 wpos = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), World );\n"
    "    float i   = dot( float3(0,0,-1), normalize(mul(float3(in_normal),World3)) );\n"
    "    VP_OUT_POSITION   = mul( wpos, ViewProjection ) + VP_TEXTURE2D( stuff, in_texcoord );\n"
    "    VP_OUT(uv)        = in_texcoord;\n"
    "    VP_OUT(color)     = float4(i,i,i,1.0);\n"
    "\n"
    "VPROG_END\n";

    rhi::ShaderSource   vp;

    vp.Construct( rhi::PROG_VERTEX, vp_src );
    Logger::Info( "\n\n====================" );
    vp.Dump();
*/
/*
    const char*  vp_src =
    "VPROG_IN_BEGIN\n"
    "    VPROG_IN_POSITION\n"
    "    VPROG_IN_NORMAL\n"
    "    VPROG_IN_TEXCOORD\n"
    "VPROG_IN_END\n"
    "\n"
    "VPROG_OUT_BEGIN\n"
    "    VPROG_OUT_POSITION\n"
    "    VPROG_OUT_TEXCOORD0(uv,2)\n"
    "    VPROG_OUT_TEXCOORD1(color,4)\n"
    "VPROG_OUT_END\n"
    "\n"
    "property float4x4 ViewProjection : shared,dynamic : ui_hidden=yes ;\n"
    "property float4x4 World : unique,dynamic : ui_hidden=yes ;\n"
    "\n"
    "VPROG_BEGIN\n"
    "\n"
    "    float3 in_pos      = VP_IN_POSITION;\n"
    "    float3 in_normal   = VP_IN_NORMAL;\n"
    "    float2 in_texcoord = VP_IN_TEXCOORD;\n"
    "    float3x3 World3 = VP_BUF_FLOAT3X3(1,0);"
    "    float4 wpos = mul( float4(in_pos.x,in_pos.y,in_pos.z,1.0), World );\n"
    "    float i   = dot( float3(0,0,-1), normalize(mul(float3(in_normal),World3)) );\n"
    "    VP_OUT_POSITION   = mul( wpos, ViewProjection );\n"
    "    VP_OUT(uv)        = in_texcoord;\n"
    "    VP_OUT(color)     = float4(i,i,i,1.0);\n"
    "\n"
    "VPROG_END\n";

    const char*  fp_src =
    "FPROG_IN_BEGIN\n"
    "FPROG_IN_TEXCOORD0(uv,2)\n"
    "FPROG_IN_TEXCOORD1(color,4)\n"
    "FPROG_IN_END\n"
    "\n"
    "FPROG_OUT_BEGIN\n"
    "    FPROG_OUT_COLOR\n"
    "FPROG_OUT_END\n"
    "\n"
    "property float4 tfactor : unique,dynamic : ;"
    "\n"
    "DECL_FP_SAMPLER2D(albedo)\n"
    "\n"
    "\n"
    "FPROG_BEGIN\n"
    "    float4  diffuse = FP_TEXTURE2D( albedo, FP_IN(uv) );\n"
    "    FP_OUT_COLOR = tfactor * FP_IN(color);\n"
    "FPROG_END\n";

    rhi::ShaderSource   vp;
    rhi::ShaderSource   fp;

    vp.Construct( rhi::PROG_VERTEX, vp_src );
    Logger::Info( "\n\n====================" );
    vp.Dump();

    fp.Construct( rhi::PROG_FRAGMENT, fp_src );
    Logger::Info( "\n\n====================" );
    fp.Dump();
*/
    inited = true;
}

void GameCore::OnAppFinished()
{
    DbgDraw::Uninitialize();
//-    rhi::Uninitialize();
}

void GameCore::OnSuspend()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif

}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}


#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
    Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
}

void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)


void
GameCore::Update( float32 timeElapsed )
{
//    sceneRenderTest->Update(timeElapsed);

    static std::vector<profiler::CounterInfo>   counter;

    if( profiler::GetAverageCounters( &counter ) )
    {
    }
    
    
    {
    int maxLen = 0;
    int x0     = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx - 12*(DbgDraw::SmallCharW+1);
    int y0     = 10;
    
    for( uint32 i=0; i!=counter.size(); ++i )
    {
        int len = strlen( counter[i].name );

        if( len > maxLen )
            maxLen = len;
    }
    x0 -= maxLen*(DbgDraw::SmallCharW+1);

    DbgDraw::SetSmallTextSize();
    for( uint32 i=0; i!=counter.size(); ++i )
    {
        char    text[128];

        memset( text, ' ', sizeof(text) );
        int l = sprintf( text, "%s", counter[i].name );
        text[l] = ' ';
        sprintf( text+maxLen, "  %5u  %u", counter[i].count, counter[i].timeUs );

//        DbgDraw::Text2D( 100, 100+i*(DbgDraw::SmallCharH+1), 0xFFFFFFFF, "%s %u", counter[i].name, counter[i].timeUs );
        DbgDraw::Text2D( x0, y0+i*(DbgDraw::SmallCharH+1), 0xFFFFFFFF, text );
    }
    }
}

void GameCore::BeginFrame()
{
}

void GameCore::DrawTank()
{
/*
    rhi::RenderPassConfig   pass_desc;

    pass_desc.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    pass_desc.colorBuffer[0].clearColor[0] = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1] = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2] = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3] = 1.0f;
    pass_desc.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;

    rhi::Handle cb[1];
    rhi::Handle pass = rhi::RenderPass::Allocate(pass_desc, 1, cb);
    rhi::RenderPass::Begin(pass);
    rhi::CommandBuffer::Begin(cb[0]);    

    float angle = 0.001f*float(SystemTimer::Instance()->AbsoluteMS()) * (30.0f*3.1415f / 180.0f);    

    float clr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Matrix4 world;
    Matrix4 view_proj;

    world.Identity();
    world.CreateRotation(Vector3(0, 1, 0), angle);    
    world.SetTranslationVector(Vector3(0, 0, 25));
    //world *= Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));

    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);


    rhi::ConstBuffer::SetConst(tank.fp_const, 0, 1, clr);
    rhi::ConstBuffer::SetConst(tank.vp_const[0], 0, 4, view_proj.data);
    rhi::ConstBuffer::SetConst(tank.vp_const[1], 0, 4, world.data);

    rhi::CommandBuffer::SetPipelineState(cb[0], tank.ps);
    rhi::CommandBuffer::SetVertexConstBuffer(cb[0], 0, tank.vp_const[0]);
    rhi::CommandBuffer::SetVertexConstBuffer(cb[0], 1, tank.vp_const[1]);
    rhi::CommandBuffer::SetFragmentConstBuffer(cb[0], 0, tank.fp_const);
    rhi::CommandBuffer::SetFragmentTexture(cb[0], 0, tank.tex);    

    for (int32 i = 0, sz = tank.vb.size(); i < sz; ++i)
    {
        rhi::CommandBuffer::SetVertexData(cb[0], tank.vb[i]);
        rhi::CommandBuffer::SetIndices(cb[0], tank.ib[i]);
        rhi::CommandBuffer::DrawIndexedPrimitive(cb[0], rhi::PRIMITIVE_TRIANGLELIST, tank.indCount[i]);
    }


    rhi::CommandBuffer::End(cb[0]);
    rhi::RenderPass::End(pass);
*/
}

void
GameCore::Draw()
{
    if( !inited )
        return;
        
//    sceneRenderTest->Render();
//    rhiDraw();
    manticoreDraw();
//    rtDraw();
//    visibilityTestDraw();
}


void
GameCore::rhiDraw()
{
    SCOPED_NAMED_TIMING("GameCore::Draw");
    //-    ApplicationCore::BeginFrame();

#define DRAW_TANK 0


#if DRAW_TANK
    DrawTank();
    return;
#endif


#define USE_SECOND_CB 0

    rhi::RenderPassConfig   pass_desc;

    pass_desc.colorBuffer[0].loadAction      = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction     = rhi::STOREACTION_STORE;
    pass_desc.colorBuffer[0].clearColor[0]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2]   = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3]   = 1.0f;
    pass_desc.depthStencilBuffer.loadAction  = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;

    rhi::Handle cb[2];
#if USE_SECOND_CB
    rhi::Handle pass = rhi::RenderPass::Allocate( pass_desc, 2, cb );
#else
    rhi::Handle pass = rhi::RenderPass::Allocate( pass_desc, 1, cb );
#endif    
    float       clr[4] = { 1.0f, 0.6f, 0.0f, 1.0f };
    

    rhi::RenderPass::Begin( pass );
    rhi::CommandBuffer::Begin( cb[0] );

#if 1
    
    rhi::ConstBuffer::SetConst( triangle.fp_const, 0, 1, clr );

    rhi::CommandBuffer::SetPipelineState( cb[0], triangle.ps );
    rhi::CommandBuffer::SetVertexData( cb[0], triangle.vb );
    rhi::CommandBuffer::SetIndices( cb[0], triangle.ib );
    rhi::CommandBuffer::SetFragmentConstBuffer( cb[0], 0, triangle.fp_const );
    rhi::CommandBuffer::DrawIndexedPrimitive( cb[0], rhi::PRIMITIVE_TRIANGLELIST, 1, 3 );
    
#else
    
    uint64  cube_t1 = SystemTimer::Instance()->AbsoluteMS();
    uint64  dt      = cube_t1 - cube_t0;
    
    cube_angle += 0.001f*float(dt) * (30.0f*3.1415f/180.0f);
    cube_t0     = cube_t1;
    
    Matrix4 world;
    Matrix4 view_proj;
    
    world.Identity();
    world.CreateRotation( Vector3(0,1,0), cube_angle );
//    world.CreateRotation( Vector3(1,0,0), cube_angle );
    world.SetTranslationVector( Vector3(0,0,5) );
    //world *= Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));
    
    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);
    
    
    rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr );
    rhi::ConstBuffer::SetConst( cube.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );

    rhi::CommandBuffer::SetPipelineState( cb[0], cube.ps );    
    rhi::CommandBuffer::SetVertexConstBuffer( cb[0], 0, cube.vp_const[0] );
    rhi::CommandBuffer::SetVertexConstBuffer( cb[0], 1, cube.vp_const[1] );
    rhi::CommandBuffer::SetFragmentConstBuffer( cb[0], 0, cube.fp_const );
    rhi::CommandBuffer::SetFragmentTexture(cb[0], 0, cube.tex);
    rhi::CommandBuffer::SetVertexData(cb[0], cube.vb);    
    rhi::CommandBuffer::DrawPrimitive( cb[0], rhi::PRIMITIVE_TRIANGLELIST, 12 );    

    #if USE_SECOND_CB
    {
        const float     w = 3.0f;
        const unsigned  n = 5;
        
        rhi::CommandBuffer::Begin( cb[1] );
        for( unsigned i=0; i!=n; ++i )
        {
            const uint32 c      = (i+1) * 0x775511; // 0x15015
            const uint8* cc     = (const uint8*)(&c);
            const float  clr2[] = { float(cc[2])/255.0f, float(cc[1])/255.0f, float(cc[0])/255.0f, 1.0f };

            world.Identity();
            world.CreateRotation( Vector3(1,0,0), cube_angle );
            world.SetTranslationVector( Vector3(-0.5f*w+float(i)*(w/float(n)),1,10) );

            rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr2 );
            rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );
    
            rhi::CommandBuffer::SetPipelineState( cb[1], cube.ps );
            rhi::CommandBuffer::SetVertexData( cb[1], cube.vb );
            rhi::CommandBuffer::SetVertexConstBuffer( cb[1], 0, cube.vp_const[0] );
            rhi::CommandBuffer::SetVertexConstBuffer( cb[1], 1, cube.vp_const[1] );
            rhi::CommandBuffer::SetFragmentConstBuffer( cb[1], 0, cube.fp_const );
            rhi::CommandBuffer::SetFragmentTexture( cb[1], 0, cube.tex );
            rhi::CommandBuffer::DrawPrimitive( cb[1], rhi::PRIMITIVE_TRIANGLELIST, 12 );
        }
        rhi::CommandBuffer::End( cb[1] );
    }
    #endif
    
#endif

    
    rhi::CommandBuffer::End( cb[0] );

    rhi::RenderPass::End( pass );

    #undef USE_SECOND_CB
}


void
GameCore::manticoreDraw()
{
SCOPED_NAMED_TIMING("app-draw");
    #define USE_SECOND_CB 1

    rhi::RenderPassConfig   pass_desc;
    float                   clr[4] = { 1.0f, 0.6f, 0.0f, 1.0f };
    Matrix4                 view;
    Matrix4                 projection;




    StatSet::ResetAll();

    projection.Identity();
    view.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);

    DbgDraw::SetScreenSize( VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx, VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy );


    {
    char    title[128] = "RHI Cube  -  ";

    switch( rhi::HostApi() )
    {
        case rhi::RHI_DX9   : strcat( title, "DX9" ); break;
        case rhi::RHI_DX11  : strcat( title, "DX11" ); break;
        case rhi::RHI_GLES2 : strcat( title, "GL" ); break;
        case rhi::RHI_METAL : strcat( title, "Metal" ); break;
    }
    DbgDraw::SetNormalTextSize();
//    DbgDraw::SetSmallTextSize();
    DbgDraw::Text2D( 10, 50, 0xFFFFFFFF, title );
    }


    pass_desc.colorBuffer[0].loadAction      = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction     = rhi::STOREACTION_NONE;
    pass_desc.colorBuffer[0].clearColor[0]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2]   = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3]   = 1.0f;
    pass_desc.depthStencilBuffer.loadAction  = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    rhi::HPacketList    pl[2];
    #if USE_SECOND_CB
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 2, pl );
    #else
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 1, pl );
    #endif


    rhi::RenderPass::Begin( pass );
    rhi::BeginPacketList( pl[0] );

#if 0
    
    rhi::Packet packet;

    packet.vertexStreamCount    = 1;
    packet.vertexStream[0]      = triangle.vb;
    packet.vertexCount          = triangle.v_cnt;
    packet.indexBuffer          = triangle.ib;
    packet.renderPipelineState  = triangle.ps;
    packet.vertexConstCount     = 0;
    packet.fragmentConstCount   = 1;
    packet.fragmentConst[0]     = triangle.fp_const;
    packet.primitiveType        = rhi::PRIMITIVE_TRIANGLELIST;
    packet.primitiveCount       = 1;

    rhi::ConstBuffer::SetConst( triangle.fp_const, 0, 1, clr );
    rhi::AddPacket( pl[0], packet );

#else

    uint64  cube_t1 = SystemTimer::Instance()->AbsoluteMS();
    uint64  dt      = cube_t1 - cube_t0;
    
    cube_angle += 0.001f*float(dt) * (30.0f*3.1415f/180.0f);
    cube_t0     = cube_t1;
    
    Matrix4 world;
    Matrix4 view_proj;
    
    world.Identity();
    world.CreateRotation( Vector3(0,1,0), cube_angle );
//    world.CreateRotation( Vector3(1,0,0), cube_angle );
    world.SetTranslationVector( Vector3(0,-0.7f,5) );
    //world *= Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));
    
    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);
    
    
    rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr );
    rhi::ConstBuffer::SetConst( cube.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );

    rhi::Packet packet;

    packet.vertexStreamCount    = 1;
    packet.vertexStream[0]      = cube.vb;
    packet.vertexLayoutUID      = cube.vb_layout;
    packet.renderPipelineState  = cube.ps;
    packet.vertexConstCount     = 2;
    packet.vertexConst[0]       = cube.vp_const[0];
    packet.vertexConst[1]       = cube.vp_const[1];
    packet.fragmentConstCount   = 1;
    packet.fragmentConst[0]     = cube.fp_const;
    packet.textureSet           = cube.texSet;
    packet.samplerState         = cube.samplerState;
    packet.primitiveType        = rhi::PRIMITIVE_TRIANGLELIST;
    packet.primitiveCount       = 12;

    rhi::UpdateConstBuffer4fv( cube.fp_const, 0, clr, 1 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[0], 0, view_proj.data, 4 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
    rhi::AddPacket( pl[0], packet );

    #if USE_SECOND_CB
    {
        const unsigned  row_cnt = 200;
        const unsigned  col_cnt = 12;
//const unsigned  row_cnt = 1;
//const unsigned  col_cnt = 2;
        const float     w       = 0.5f * float(col_cnt);
        
        rhi::BeginPacketList( pl[1] );
        for( unsigned z=0; z!=row_cnt; ++z )
        {
            for( unsigned i=0; i!=col_cnt; ++i )
            {
                const uint32 c      = (z*row_cnt+i+1) * 0x775511; // 0x15015
                const uint8* cc     = (const uint8*)(&c);
                const float  clr2[] = { float(cc[2])/255.0f, float(cc[1])/255.0f, float(cc[0])/255.0f, 1.0f };

START_NAMED_TIMING("app.cb--upd");
                world.Identity();
                world.CreateRotation( Vector3(1,0,0), cube_angle );
                world.SetTranslationVector( Vector3(-0.5f*w+float(i)*(w/float(col_cnt)),1-z*0.4f,10+float(z)*w) );

                rhi::UpdateConstBuffer4fv( cube.fp_const, 0, clr2, 1 );
    //            rhi::UpdateConstBuffer( cube.vp_const[0], 0, view_proj.data, 4 );
                rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
STOP_NAMED_TIMING("app.cb--upd");
                rhi::AddPacket( pl[1], packet );
            }
        }
        rhi::EndPacketList( pl[1] );
    }
    #endif

#endif

    DbgDraw::FlushBatched( pl[0], view, projection );

    rhi::EndPacketList( pl[0] );

    rhi::RenderPass::End( pass );

    #undef USE_SECOND_CB
}

void
GameCore::visibilityTestDraw()
{
    rhi::RenderPassConfig       pass_desc;
    float                       clr[4]                  = { 1.0f, 0.6f, 0.0f, 1.0f };
    Matrix4                     view;
    Matrix4                     projection;
    static bool                 visiblityTestDone       = false;
    static int                  visiblityTestRepeatTTW  = 0;
    static rhi::HQueryBuffer    visibilityBuffer        (rhi::InvalidHandle);


    StatSet::ResetAll();

    projection.Identity();
    view.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);

    DbgDraw::SetScreenSize( VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx, VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy );

    {
    char    title[128] = "RHI Cube  -  ";

    switch( rhi::HostApi() )
    {
        case rhi::RHI_DX9   : strcat( title, "DX9" ); break;
        case rhi::RHI_DX11  : strcat( title, "DX11" ); break;
        case rhi::RHI_GLES2 : strcat( title, "GL" ); break;
        case rhi::RHI_METAL : strcat( title, "Metal" ); break;
    }
    DbgDraw::SetNormalTextSize();
//    DbgDraw::SetSmallTextSize();
    DbgDraw::Text2D( 10, 50, 0xFFFFFFFF, title );
    
    if( visiblityTestDone )    
    {
        for( unsigned i=0; i!=2; ++i )
        {
            if( rhi::QueryIsReady( visibilityBuffer, i ) )
            {
                DbgDraw::Text2D( 10, 80+i*(DbgDraw::NormalCharH+1), 0xFFFFFFFF, "obj#%u = %i", i, rhi::QueryValue( visibilityBuffer, i ) );
            }
            else
            {
                DbgDraw::Text2D( 10, 80+i*(DbgDraw::NormalCharH+1), 0xFFFFFFFF, "obj#%u = not ready", i );
            }
        }
    }
    }

    pass_desc.colorBuffer[0].loadAction      = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction     = rhi::STOREACTION_NONE;
    pass_desc.colorBuffer[0].clearColor[0]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2]   = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3]   = 1.0f;
    pass_desc.depthStencilBuffer.loadAction  = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    if( !visiblityTestDone )
    {
        if( !visibilityBuffer.IsValid() )
        {
            visibilityBuffer = rhi::CreateQueryBuffer( 16 );
        }

        pass_desc.queryBuffer = visibilityBuffer;
    }

    rhi::HPacketList    pl[2];
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 1, pl );


    rhi::RenderPass::Begin( pass );
    rhi::BeginPacketList( pl[0] );

    uint64  cube_t1 = SystemTimer::Instance()->AbsoluteMS();
    uint64  dt      = cube_t1 - cube_t0;
    
    cube_angle += 0.001f*float(dt) * (30.0f*3.1415f/180.0f);
    cube_t0     = cube_t1;
    
    Matrix4 world;
    Matrix4 view_proj;
    
    world.Identity();
    world.CreateRotation( Vector3(0,1,0), cube_angle );
//    world.CreateRotation( Vector3(1,0,0), cube_angle );
    world.SetTranslationVector( Vector3(0,0,5) );
    //world *= Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));
    
    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);
    
    
    rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr );
    rhi::ConstBuffer::SetConst( cube.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );

    rhi::Packet packet;

    packet.vertexStreamCount    = 1;
    packet.vertexStream[0]      = cube.vb;
    packet.vertexLayoutUID      = cube.vb_layout;
//-    packet.indexBuffer          = rhi::InvalidHandle;
    packet.renderPipelineState  = cube.ps;
    packet.vertexConstCount     = 2;
    packet.vertexConst[0]       = cube.vp_const[0];
    packet.vertexConst[1]       = cube.vp_const[1];
    packet.fragmentConstCount   = 1;
    packet.fragmentConst[0]     = cube.fp_const;
    packet.textureSet           = cube.texSet;
    packet.samplerState         = cube.samplerState;
    packet.primitiveType        = rhi::PRIMITIVE_TRIANGLELIST;
    packet.primitiveCount       = 12;

    rhi::UpdateConstBuffer4fv( cube.fp_const, 0, clr, 1 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[0], 0, view_proj.data, 4 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
    if( !visiblityTestDone )
        packet.queryIndex = 0;
    rhi::AddPacket( pl[0], packet );


    world.SetTranslationVector( Vector3(0,0,20) );
    rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
    if( !visiblityTestDone )
        packet.queryIndex = 1;
    rhi::AddPacket( pl[0], packet );




    DbgDraw::FlushBatched( pl[0], view, projection );

    rhi::EndPacketList( pl[0] );

    rhi::RenderPass::End( pass );


    
    if( !visiblityTestDone )
    {
        visiblityTestDone       = true;
        visiblityTestRepeatTTW  = 5;
    }

    if( visiblityTestDone )
    {
        if( --visiblityTestRepeatTTW < 0 )
            visiblityTestDone = false;
    }
}


void
GameCore::rtDraw()
{
    #define USE_SECOND_CB   1
    #define USE_RT          1


    // draw scene into render-target
    {

    rhi::RenderPassConfig   pass_desc;
    float                   clr[4] = { 1.0f, 0.6f, 0.0f, 1.0f };

    #if USE_RT
    pass_desc.colorBuffer[0].texture         = rtColor;
    pass_desc.depthStencilBuffer.texture     = rtDepthStencil;
    #endif
    pass_desc.colorBuffer[0].loadAction      = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction     = rhi::STOREACTION_STORE;
    pass_desc.colorBuffer[0].clearColor[0]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2]   = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3]   = 1.0f;
    pass_desc.depthStencilBuffer.loadAction  = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;

    rhi::HPacketList    pl[2];
    #if USE_SECOND_CB
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 2, pl );
    #else
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 1, pl );
    #endif


    rhi::RenderPass::Begin( pass );
    rhi::BeginPacketList( pl[0] );

    uint64  cube_t1 = SystemTimer::Instance()->AbsoluteMS();
    uint64  dt      = cube_t1 - cube_t0;
    
    cube_angle += 0.001f*float(dt) * (30.0f*3.1415f/180.0f);
    cube_t0     = cube_t1;
    
    Matrix4 world;
    Matrix4 view_proj;
    
    world.Identity();
    world.CreateRotation( Vector3(0,1,0), cube_angle );
//    world.CreateRotation( Vector3(1,0,0), cube_angle );
    world.SetTranslationVector( Vector3(0,0,5) );
    //world *= Matrix4::MakeScale(Vector3(0.5f, 0.5f, 0.5f));
    
    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy), 1.0f, 1000.0f);
    
    
    rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr );
    rhi::ConstBuffer::SetConst( cube.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );

    rhi::Packet packet;

    packet.vertexStreamCount    = 1;
    packet.vertexStream[0]      = cube.vb;
//-    packet.indexBuffer          = rhi::InvalidHandle;
    packet.renderPipelineState  = cube.ps;
    packet.vertexConstCount     = 2;
    packet.vertexConst[0]       = cube.vp_const[0];
    packet.vertexConst[1]       = cube.vp_const[1];
    packet.fragmentConstCount   = 1;
    packet.fragmentConst[0]     = cube.fp_const;
    packet.textureSet           = cube.texSet;
    packet.samplerState         = cube.samplerState;
    packet.primitiveType        = rhi::PRIMITIVE_TRIANGLELIST;
    packet.primitiveCount       = 12;

    rhi::UpdateConstBuffer4fv( cube.fp_const, 0, clr, 1 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[0], 0, view_proj.data, 4 );
    rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
    rhi::AddPacket( pl[0], packet );

    #if USE_SECOND_CB
    {
        const float     w       = 5.0f;
        const unsigned  row_cnt = 30;
        const unsigned  col_cnt = 7;
        
        rhi::BeginPacketList( pl[1] );
        for( unsigned z=0; z!=row_cnt; ++z )
        {
            for( unsigned i=0; i!=col_cnt; ++i )
            {
                const uint32 c      = (z*col_cnt+i+1) * 0x775511; // 0x15015
                const uint8* cc     = (const uint8*)(&c);
                const float  clr2[] = { float(cc[2])/255.0f, float(cc[1])/255.0f, float(cc[0])/255.0f, 1.0f };

                world.Identity();
                world.CreateRotation( Vector3(1,0,0), cube_angle );
                world.SetTranslationVector( Vector3(-0.5f*w+float(i)*(w/float(col_cnt)),1,10+float(z)*w) );

                rhi::UpdateConstBuffer4fv( cube.fp_const, 0, clr2, 1 );
    //            rhi::UpdateConstBuffer( cube.vp_const[0], 0, view_proj.data, 4 );
                rhi::UpdateConstBuffer4fv( cube.vp_const[1], 0, world.data, 4 );
                rhi::AddPacket( pl[1], packet );
            }
        }
        rhi::EndPacketList( pl[1] );
    }
    #endif

    rhi::EndPacketList( pl[0] );
    rhi::RenderPass::End( pass );
    }

    
    // draw render-target contents on-screen
    #if USE_RT
    {
    rhi::RenderPassConfig   pass_desc;

    pass_desc.colorBuffer[0].loadAction      = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction     = rhi::STOREACTION_STORE;
    pass_desc.colorBuffer[0].clearColor[0]   = 0.15f;
    pass_desc.colorBuffer[0].clearColor[1]   = 0.15f;
    pass_desc.colorBuffer[0].clearColor[2]   = 0.25f;
    pass_desc.colorBuffer[0].clearColor[3]   = 1.0f;
    pass_desc.depthStencilBuffer.loadAction  = rhi::LOADACTION_CLEAR;
    pass_desc.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;

    rhi::HPacketList    pl;
    rhi::HRenderPass    pass = rhi::AllocateRenderPass( pass_desc, 1, &pl );


    rhi::RenderPass::Begin( pass );
    rhi::BeginPacketList( pl );

    Matrix4 world;
    Matrix4 view_proj;
    float   ratio   = float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx) / float(VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy);
    
    world = Matrix4::MakeRotation( Vector3(0,1,0), (30.0f*3.1415f/180.0f) ) * Matrix4::MakeScale(Vector3(ratio,1,1));
    world.SetTranslationVector( Vector3(-2,0,15) );
    
    view_proj.Identity();
    view_proj.BuildProjectionFovLH(75.0f, ratio, 1.0f, 1000.0f);
        
    rhi::ConstBuffer::SetConst( rtQuad.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( rtQuad.vp_const[1], 0, 4, world.data );

    rhi::AddPacket( pl, rtQuadBatch );

    
    rhi::EndPacketList( pl );
    rhi::RenderPass::End( pass );
    }
    #endif

    #undef USE_SECOND_CB
    #undef USE_RT
}



void GameCore::EndFrame()
{
SCOPED_NAMED_TIMING("GameCore::EndFrame");    
    rhi::Present();

    
    // rendering stats

    {
    const unsigned id[] = 
    {
        rhi::stat_DIP,
        rhi::stat_DP,
        rhi::stat_SET_PS,
        rhi::stat_SET_CB,
        rhi::stat_SET_TEX
    };
    unsigned        max_nl = 0;
    
    for( unsigned i=0; i!=countof(id); ++i )
    {
        unsigned    l = strlen( StatSet::StatFullName(id[i]) );

        if( l > max_nl  )
            max_nl = l;
    }

    const int   w   = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx;
    const int   h   = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy;
    const int   x0  = w - 230; 
    const int   x1  = x0 + (max_nl+1)*(DbgDraw::SmallCharW+1);
    const int   lh  = DbgDraw::SmallCharH + 2;
    int         y   = h - 200;
    
    DbgDraw::SetSmallTextSize();
    for( unsigned i=0; i!=countof(id); ++i,y+=lh )
    {
//        const uint32_t  clr = (i&0x1) ? Color4f(0.4f,0.4f,0.8f,1) : Color4f(0.4f,0.4f,1.0f,1);
        const uint32_t  clr = 0xFFFFFFFF;

//        DbgDraw::FilledRect2D( x0-8, y, x0+50*(DbgDraw::SmallCharW+1), y+lh );
        DbgDraw::Text2D( x0, y, clr, StatSet::StatFullName(id[i]) );
        DbgDraw::Text2D( x1, y, clr, "= %u", StatSet::StatValue(id[i]) );
    }
    }

}


