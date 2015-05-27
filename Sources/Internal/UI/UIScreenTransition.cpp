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



#include "UI/UIScreenTransition.h"
#include "Render/RenderHelper.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/Image.h"
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
Sprite * UIScreenTransition::renderTargetPrevScreen = 0;
Sprite * UIScreenTransition::renderTargetNextScreen = 0;

UniqueHandle UIScreenTransition::alphaClearStateHandle = InvalidUniqueHandle;

UIScreenTransition::UIScreenTransition()
{
    duration = 0.7f;
    interpolationFunc = Interpolation::GetFunction(Interpolation::EASY_IN_EASY_OUT);
    SetFillBorderOrder(UIScreen::FILL_BORDER_AFTER_DRAW);

}

UIScreenTransition::~UIScreenTransition()
{
}

void UIScreenTransition::CreateRenderTargets()
{
#if RHI_COMPLETE
    if (renderTargetPrevScreen || renderTargetNextScreen)
    {
        Logger::FrameworkDebug("Render targets already created");
        return;
    }
    /*copy of default 3d blend with alpha write only - to minimize state changes*/
    alphaClearStateHandle = RenderManager::Instance()->SubclassRenderState(RenderState::RENDERSTATE_3D_BLEND, RenderStateData::STATE_DEPTH_WRITE | RenderStateData::STATE_DEPTH_TEST | RenderStateData::STATE_CULL | RenderStateData::STATE_COLORMASK_ALPHA);

    uint32 width = (uint32)VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx;
    uint32 height = (uint32)VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy;

    Texture * tex1 = Texture::CreateFBO(width, height, FORMAT_RGB565, Texture::DEPTH_RENDERBUFFER);
    Texture * tex2 = Texture::CreateFBO(width, height, FORMAT_RGB565, Texture::DEPTH_RENDERBUFFER);

    renderTargetPrevScreen = Sprite::CreateFromTexture(tex1, 0, 0, (float32)width, (float32)height, true);
    renderTargetNextScreen = Sprite::CreateFromTexture(tex2, 0, 0, (float32)width, (float32)height, true);

    SafeRelease(tex1);
    SafeRelease(tex2);
#endif //RHI_COMPLETE
}

void UIScreenTransition::ReleaseRenderTargets()
{
    SafeRelease(renderTargetPrevScreen);
    SafeRelease(renderTargetNextScreen);
}

void UIScreenTransition::StartTransition(UIScreen * _prevScreen, UIScreen * _nextScreen)
{
#if RHI_COMPLETE
    CreateRenderTargets();
    nextScreen = _nextScreen;
    prevScreen = _prevScreen;

    Rect oldViewport = RenderManager::Instance()->GetViewport();
    
    RenderSystem2D::Instance()->Flush();

    Texture * textureTargetPrev = renderTargetPrevScreen->GetTexture();
    RenderManager::Instance()->SetRenderTarget(textureTargetPrev);
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)textureTargetPrev->GetWidth(), (float32)textureTargetPrev->GetHeight()));
    RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);    
    RenderManager::Instance()->Clear(Color(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

    RenderSystem2D::Instance()->Setup2DMatrices();

    if (prevScreen)
    {
        prevScreen->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());

        if (prevScreen->IsOnScreen())
            prevScreen->SystemWillBecomeInvisible();

        prevScreen->SystemWillDisappear();
        // prevScreen->UnloadResources();
        if (prevScreen->GetGroupId() != nextScreen->GetGroupId())
            prevScreen->UnloadGroup();
        prevScreen->SystemDidDisappear();

        SafeRelease(prevScreen);
    }

    RenderSystem2D::Instance()->Flush();

    /*clear alpha*/
    RenderManager::Instance()->SetRenderState(alphaClearStateHandle);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->ClearWithColor(0.0, 0.0, 0.0, 1.0);

    nextScreen->LoadGroup();
    nextScreen->SystemWillAppear();

    //
    Texture * textureTargetNext = renderTargetNextScreen->GetTexture();
    RenderManager::Instance()->SetRenderTarget(textureTargetNext);
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)textureTargetNext->GetWidth(), (float32)textureTargetNext->GetHeight()));
    RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->Clear(Color(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

    RenderSystem2D::Instance()->Setup2DMatrices();

    float32 timeElapsed = SystemTimer::FrameDelta();
    nextScreen->SystemUpdate(timeElapsed);
    nextScreen->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());

    RenderSystem2D::Instance()->Flush();

    /*clear alpha*/
    RenderManager::Instance()->SetRenderState(alphaClearStateHandle);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->ClearWithColor(0.0, 0.0, 0.0, 1.0);
    
    RenderManager::Instance()->SetRenderTarget(0);
    RenderManager::Instance()->SetViewport(oldViewport);
    RenderSystem2D::Instance()->Setup2DMatrices();

    //  Debug images. Left here for future bugs :)
    //    Image * image = renderTargetPrevScreen->GetTexture()->CreateImageFromMemory();
    //    ImageLoader::Save(image, "~doc:/render_target_prev.png");
    //    SafeRelease(image);
    //
    //    Image * image2 = renderTargetNextScreen->GetTexture()->CreateImageFromMemory();
    //    ImageLoader::Save(image2, "~doc:/render_target_next.png");
    //    SafeRelease(image2);

    currentTime = 0;
#endif // RHI_COMPLETE
}

void UIScreenTransition::Update(float32 timeElapsed)
{
    currentTime += timeElapsed;
    normalizedTime = interpolationFunc(currentTime / duration);
    if (currentTime >= duration)
    {
        currentTime = duration;
        UIControlSystem::Instance()->ReplaceScreen(nextScreen);
        nextScreen->SystemDidAppear();
        if (nextScreen->IsOnScreen())
            nextScreen->SystemWillBecomeVisible();
        ReleaseRenderTargets();
        // go to next screen
        UIControlSystem::Instance()->UnlockInput();
        UIControlSystem::Instance()->UnlockSwitch();

        /*
            Right now we are in update so when we change control we miss Update for new screen
            Here we call update control to make calls to update / draw sequential and avoid problem with missing Update
            We pass current timeElapsed because we miss current frame time
            */
        nextScreen->SystemUpdate(timeElapsed); // 
    }
}

void UIScreenTransition::Draw(const UIGeometricData &geometricData)
{
    RenderSystem2D::Instance()->Setup2DMatrices();

    Sprite::DrawState drawState;
    drawState.SetMaterial(RenderHelper::DEFAULT_2D_TEXTURE_MATERIAL);

    drawState.SetScale(0.5f, 1.0f);
    drawState.SetPosition(0, 0);

    RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState);

    drawState.SetScale(0.5f, 1.0f);
    drawState.SetPosition((VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dx) / 2.0f, 0);

    RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState);
}

void UIScreenTransition::SetDuration(float32 timeInSeconds)
{
    duration = timeInSeconds;
};

bool UIScreenTransition::IsLoadingTransition()
{
    return false;
}

};

