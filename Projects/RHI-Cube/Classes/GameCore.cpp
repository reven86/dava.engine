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
    #include "../rhi_ShaderCache.h"


using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
    rhi::Initialize();
    rhi::ShaderCache::Initialize();

    triangle.vb = rhi::VertexBuffer::Create( 3*sizeof(VertexP) );
    triangle.ib = rhi::IndexBuffer::Create( 3*sizeof(uint16) );
    
    VertexP*    v = (VertexP*)rhi::VertexBuffer::Map( triangle.vb, 0, 3*sizeof(VertexP) );

    if( v )
    {
        v[0].x = -0.2f;
        v[0].y = 0.0f;
        v[0].z = 0.0f;
        
        v[1].x = 0.0f;
        v[1].y = 0.2f;
        v[1].z = 0.0f;
        
        v[2].x = 0.2f;
        v[2].y = 0.0f;
        v[2].z = 0.0f;
        
        rhi::VertexBuffer::Unmap( triangle.vb );
    }

    uint16  i[3] = { 0, 1, 2 };

    rhi::IndexBuffer::Update( triangle.ib, i, 0, 3*sizeof(uint16) );

    rhi::ShaderCache::UpdateProg
    ( 
        rhi::PROG_VERTEX, FastName("vp-simple"),
        "attribute vec4 attr_position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(attr_position.x,attr_position.y,attr_position.z,1.0);\n"
        "}\n"
    );
    rhi::ShaderCache::UpdateProg
    ( 
        rhi::PROG_FRAGMENT, FastName("fp-simple"),
        "uniform vec4 FP_Buffer0[4];\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = FP_Buffer0[0];\n"
        "}\n"
    );


    rhi::PipelineState::Descriptor  psDesc;

    psDesc.vertexLayout.Clear();
    psDesc.vertexLayout.AddElement( rhi::vsPosition, 0, rhi::vdtFloat, 3 );
    psDesc.vprogUid = FastName("vp-simple");
    psDesc.fprogUid = FastName("fp-simple");

    triangle.ps    = rhi::PipelineState::Create( psDesc );
    triangle.fp_cb = rhi::PipelineState::CreateFProgConstBuffer( triangle.ps, 0 );
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
    ApplicationCore::BeginFrame();

    rhi::Handle cb    = rhi::CommandBuffer::Default();
    float       clr[4] = { 1.0f, 0.6f, 0.0f, 1.0f };

    rhi::CommandBuffer::Begin( cb );    
    rhi::CommandBuffer::Clear( cb );

    rhi::ConstBuffer::SetConst( triangle.fp_cb, 0, 1, clr );

    rhi::CommandBuffer::SetVertexData( cb, triangle.vb );
    rhi::CommandBuffer::SetIndices( cb, triangle.ib );
    rhi::CommandBuffer::SetPipelineState( cb, triangle.ps );
    rhi::CommandBuffer::SetFragmentConstBuffer( cb, 0, triangle.fp_cb );
    rhi::CommandBuffer::DrawIndexedPrimitive( cb, rhi::PRIMITIVE_TRIANGLELIST, 1 );
    
    rhi::CommandBuffer::End( cb );
}

void GameCore::EndFrame()
{
    rhi::Present();
}


