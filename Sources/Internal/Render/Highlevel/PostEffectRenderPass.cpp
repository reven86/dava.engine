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

#include "Render/Highlevel/PostEffectRenderPass.h"
#include "Render/Highlevel/RenderPassManager.h"
#include "Render/ShaderCache.h"
#include "Render/Image.h"
#include "Render/LibPngHelpers.h"

namespace DAVA
{

FastName PostEffectRenderPass::SHADER_NAME("~res:/Materials/Shaders/PostEffect/posteffect");

PostEffectRenderPass::PostEffectRenderPass(RenderSystem * renderSystem, const FastName & name, RenderPassID id)
:   RenderPass(renderSystem, name, id),
    currentViewport(Rect(-1.f, -1.f, -1.f, -1.f)),
    renderTexture(0),
    rdo(0),
    textureState(InvalidUniqueHandle)
{
    renderTarget = Sprite::Create("");

    shader = SafeRetain(ShaderCache::Instance()->Get(SHADER_NAME, FastNameSet()));
}

PostEffectRenderPass::~PostEffectRenderPass()
{
    SafeRelease(renderTarget);
    SafeRelease(renderTexture);
    SafeRelease(shader);
    SafeRelease(rdo);
}

void PostEffectRenderPass::Init()
{
    SafeRelease(rdo);
    SafeRelease(renderTexture);
    if(textureState != InvalidUniqueHandle)
    {
        RenderManager::Instance()->ReleaseTextureState(textureState);
    }

    renderTexture = (Texture::CreateFBO((int32)ceilf(currentViewport.dx), (int32)ceilf(currentViewport.dy), FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER));
    renderTexture->SetMinMagFilter(Texture::FILTER_NEAREST, Texture::FILTER_LINEAR);
    renderTarget->InitFromTexture(renderTexture, 0, 0, currentViewport.dx, currentViewport.dy, -1, -1, true);
    TextureStateData textureStateData;
    textureStateData.SetTexture(0, renderTexture);

    textureState = RenderManager::Instance()->CreateTextureState(textureStateData);

    rdo = new RenderDataObject();
    Vector3 vert3[4] = {Vector3(-1.f, 1.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector3(-1.f, -1.f, 1.f), Vector3(1.f, -1.f, 1.f)};

    float32 texCoordW = currentViewport.dx/renderTexture->GetWidth();
    float32 texCoordH = currentViewport.dy/renderTexture->GetHeight();
    Vector2 texc0[4] = {Vector2(0.f, 0.f), Vector2(texCoordW, 0.f), Vector2(0.f, texCoordH), Vector2(texCoordW, texCoordH) };

    for(int32 i = 0; i < 4; ++i)
    {
        vertices[i*3] = vert3[i].x;
        vertices[i*3+1] = vert3[i].y;
        vertices[i*3+2] = vert3[i].z;
        texCoords0[i*2] = texc0[i].x;
        texCoords0[i*2+1] = texc0[i].y;
    }

    rdo->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, vertices);
    rdo->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, texCoords0);
}

void PostEffectRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
    RenderManager * rm = RenderManager::Instance();
    const Rect & newViewport = rm->GetViewport();
    if(currentViewport != newViewport)
    {
        currentViewport = newViewport;
        Init();
    }

    RenderManager::Instance()->SetRenderTarget(renderTarget);
    RenderManager::Instance()->SetViewport(currentViewport, true);
    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->Clear(Color(0.f, 0.f, 1.f, 1.f), 1.f, 0);
    camera->SetupDynamicParameters();

    RenderPass * forwardPass = renderSystem->GetRenderPassManager()->GetRenderPass(RENDER_PASS_FORWARD_ID);
    forwardPass->Draw(camera, renderSystem);

    RenderManager::Instance()->RestoreRenderTarget();
    RenderManager::Instance()->SetViewport(currentViewport, true);

    RenderManager::Instance()->SetTextureState(textureState);
    RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->SetRenderData(rdo);
    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_OPAQUE);
    RenderManager::Instance()->FlushState();

    RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
}



}