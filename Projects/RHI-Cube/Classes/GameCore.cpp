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
    
    #include "../rhi_Base.h"
    #include "../RHI/rhi_ShaderCache.h"

    #include "Base/Profiler.h"


using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void    
GameCore::SetupTriangle()
{
    triangle.vb = rhi::VertexBuffer::Create( 3*sizeof(VertexP) );
    triangle.ib = rhi::IndexBuffer::Create( 3*sizeof(uint16) );
    
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

/*
"precision highp float;\n"
        "attribute vec4 attr_position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(attr_position.x,attr_position.y,attr_position.z,1.0);\n"
        "}\n"
*/    
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

/*
        "precision highp float;\n"
#if DV_USE_UNIFORMBUFFER_OBJECT
        "uniform FP_Buffer0_Block { vec4 FP_Buffer0[4]; };\n"
#else
        "uniform vec4 FP_Buffer0[4];\n"
#endif
        "void main()\n"
        "{\n"
        "    gl_FragColor = FP_Buffer0[0];\n"
        "}\n"
*/    
    );


    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement( rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3 );
    psDesc.vprogUid = FastName("vp-simple");
    psDesc.fprogUid = FastName("fp-simple");

    triangle.ps       = rhi::PipelineState::Create( psDesc );
    triangle.fp_const = rhi::PipelineState::CreateFProgConstBuffer( triangle.ps, 0 );
}


void
GameCore::SetupCube()
{
    cube.vb = rhi::VertexBuffer::Create( 3*2*6*sizeof(VertexPNT) );
    cube.ib = rhi::InvalidHandle;

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

    rhi::VertexBuffer::Update( cube.vb, v, 0, sizeof(v) );


    cube.tex = rhi::Texture::Create( 128, 128, rhi::TEXTURE_FORMAT_A8R8G8B8 );
    
    uint8*  tex = (uint8*)(rhi::Texture::Map( cube.tex ));

    if( tex )
    {
        uint8   color1[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        uint8   color2[4] = { 0x80, 0x80, 0x80, 0x80 };
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
        "DECL_SAMPLER2D(0)\n"
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

    cube.ps          = rhi::PipelineState::Create( psDesc );
    cube.vp_const[0] = rhi::PipelineState::CreateVProgConstBuffer( cube.ps, 0 );
    cube.vp_const[1] = rhi::PipelineState::CreateVProgConstBuffer( cube.ps, 1 );
    cube.fp_const    = rhi::PipelineState::CreateFProgConstBuffer( cube.ps, 0 );

    cube_t0     = SystemTimer::Instance()->AbsoluteMS();
    cube_angle  = 0;
}

void GameCore::OnAppStarted()
{
    rhi::Initialize();
    rhi::ShaderCache::Initialize();
    
    SetupTriangle();
    SetupCube();
}

void GameCore::OnAppFinished()
{
    rhi::Uninitialize();
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


void GameCore::BeginFrame()
{
}

void
GameCore::Draw()
{
    SCOPED_NAMED_TIMING("GameCore::Draw");
    //-    ApplicationCore::BeginFrame();

#define USE_SECOND_CB 1

    rhi::RenderPassConfig   pass_desc;

    pass_desc.colorBuffer[0].loadAction     = rhi::LOADACTION_CLEAR;
    pass_desc.colorBuffer[0].storeAction    = rhi::STOREACTION_STORE;
    pass_desc.colorBuffer[0].clearColor[0]  = 0.25f;
    pass_desc.colorBuffer[0].clearColor[1]  = 0.25f;
    pass_desc.colorBuffer[0].clearColor[2]  = 0.35f;
    pass_desc.colorBuffer[0].clearColor[3]  = 1.0f;
    pass_desc.depthBuffer.loadAction        = rhi::LOADACTION_CLEAR;
    pass_desc.depthBuffer.storeAction       = rhi::STOREACTION_STORE;

    rhi::Handle cb[2];
#if USE_SECOND_CB
    rhi::Handle pass = rhi::RenderPass::Allocate( pass_desc, 2, cb );
#else
    rhi::Handle pass = rhi::RenderPass::Allocate( pass_desc, 1, cb );
#endif    
    float       clr[4] = { 1.0f, 0.6f, 0.0f, 1.0f };
    

    rhi::RenderPass::Begin( pass );
    rhi::CommandBuffer::Begin( cb[0] );

#if 0
    
    rhi::ConstBuffer::SetConst( triangle.fp_const, 0, 1, clr );
    
    rhi::CommandBuffer::SetPipelineState( cb, triangle.ps );
    rhi::CommandBuffer::SetVertexData( cb, triangle.vb );
    rhi::CommandBuffer::SetIndices( cb, triangle.ib );
    rhi::CommandBuffer::SetFragmentConstBuffer( cb, 0, triangle.fp_const );
    rhi::CommandBuffer::DrawIndexedPrimitive( cb, rhi::PRIMITIVE_TRIANGLELIST, 1 );
    
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
    view_proj.Identity();
    view_proj.BuildProjectionFovLH( 75.0f, Core::Instance()->GetPhysicalScreenWidth()/Core::Instance()->GetPhysicalScreenHeight(), 1.0f,1000.0f );
    
    
    rhi::ConstBuffer::SetConst( cube.fp_const, 0, 1, clr );
    rhi::ConstBuffer::SetConst( cube.vp_const[0], 0, 4, view_proj.data );
    rhi::ConstBuffer::SetConst( cube.vp_const[1], 0, 4, world.data );

    rhi::CommandBuffer::SetPipelineState( cb[0], cube.ps );
    rhi::CommandBuffer::SetVertexData( cb[0], cube.vb );
    rhi::CommandBuffer::SetVertexConstBuffer( cb[0], 0, cube.vp_const[0] );
    rhi::CommandBuffer::SetVertexConstBuffer( cb[0], 1, cube.vp_const[1] );
    rhi::CommandBuffer::SetFragmentConstBuffer( cb[0], 0, cube.fp_const );
    rhi::CommandBuffer::SetFragmentTexture( cb[0], 0, cube.tex );
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
}

void GameCore::EndFrame()
{
SCOPED_NAMED_TIMING("GameCore::EndFrame");    
    rhi::Present();
}


