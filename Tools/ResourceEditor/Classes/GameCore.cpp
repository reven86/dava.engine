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

#include "FileSystem/ResourceArchive.h"
#include "StringConstants.h"
#include "version.h"

using namespace DAVA;


GameCore::GameCore()
{ }

GameCore::~GameCore()
{ }

void GameCore::OnAppStarted()
{
    Renderer::GetOptions()->SetOption(RenderOptions::LAYER_OCCLUSION_STATS, true);
    DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    UIControlSystem::Instance()->SetClearColor(Color(.3f, .3f, .3f, 1.f));
}

void GameCore::OnAppFinished()
{ }

void GameCore::OnSuspend()
{
	//prevent going to suspend
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
	//prevent going to background
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
}
static float t = 0;
void GameCore::Update(float32 timeElapsed)
{
	ApplicationCore::Update(timeElapsed);
    t += timeElapsed;
}

void GameCore::Draw()
{
    float s = fabs(sinf(t));
    static Texture* rt = Texture::CreateFBO(2048, 2048, PixelFormat::FORMAT_RGBA8888, true);
    RenderSystem2D::RenderTargetPassDescriptor descr;
    descr.colorAttachment = rt->handle;
    descr.depthAttachment = rt->handleDepthStencil;
    descr.clearColor = Color(0.0f, 1.0f, 1.0f, 1.0f);
    descr.priority = -100;
    RenderSystem2D::Instance()->SetMainTargetDescriptor(descr);
    ApplicationCore::Draw();

    RenderSystem2D::Instance()->SetMainTargetDescriptor(RenderSystem2D::RenderTargetPassDescriptor());
    rhi::Viewport viewport;
    viewport.x = viewport.y = 0U;
    viewport.width = (uint32)Renderer::GetFramebufferWidth();
    viewport.height = (uint32)Renderer::GetFramebufferHeight();
    RenderHelper::CreateClearPass(rhi::HTexture(), rhi::HTexture(), PRIORITY_CLEAR, Color::White, viewport);
    RenderSystem2D::Instance()->BeginFrame();
    Size2i sz = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
    Vector2 of = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
    float ss = 0.5f * sz.dx * s;
    RenderSystem2D::Instance()->DrawTexture(rt, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, Rect(of.x + 0.5f * sz.dx - ss, of.y, 2 * ss, of.y + sz.dy), Rect(0, 0, float(sz.dx) / 2048.0f, float(sz.dy) / 2048.0f));
    //RenderSystem2D::Instance()->DrawTexture(rt, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, Rect(of.x , of.y, sz.dx , sz.dy), Rect(0, 0, float(sz.dx) / 2048.0f, float(sz.dy) / 2048.0f));
    RenderSystem2D::Instance()->EndFrame();
}


