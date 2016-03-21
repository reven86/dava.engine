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

#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"
#include "UI/UIScreenshoter.h"

namespace DAVA
{
UIScreenTransition::UIScreenTransition()
{
    interpolationFunc = Interpolation::GetFunction(Interpolation::EASY_IN_EASY_OUT);
    SetFillBorderOrder(UIScreen::FILL_BORDER_AFTER_DRAW);
}

UIScreenTransition::~UIScreenTransition()
{
    DVASSERT(renderTargetPrevScreen == nullptr && renderTargetNextScreen == nullptr);
}

void UIScreenTransition::CreateRenderTargets()
{
    if (renderTargetPrevScreen || renderTargetNextScreen)
    {
        Logger::FrameworkDebug("Render targets already created");
        return;
    }

    VirtualCoordinatesSystem* vcs = VirtualCoordinatesSystem::Instance();

    Size2i physicalTargetSize = vcs->GetPhysicalScreenSize();

    uint32 width = physicalTargetSize.dx;
    uint32 height = physicalTargetSize.dy;

    Texture* tex1 = Texture::CreateFBO(width, height, FORMAT_RGB565, true);
    Texture* tex2 = Texture::CreateFBO(width, height, FORMAT_RGB565, true);

    renderTargetPrevScreen = Sprite::CreateFromTexture(tex1, 0, 0, static_cast<float32>(width), static_cast<float32>(height), true);
    renderTargetNextScreen = Sprite::CreateFromTexture(tex2, 0, 0, static_cast<float32>(width), static_cast<float32>(height), true);

    SafeRelease(tex1);
    SafeRelease(tex2);
}

void UIScreenTransition::ReleaseRenderTargets()
{
    SafeRelease(renderTargetPrevScreen);
    SafeRelease(renderTargetNextScreen);
}

void UIScreenTransition::StartTransition()
{
    currentTime = 0.0f;
    complete = false;

    CreateRenderTargets();
}

void UIScreenTransition::SetSourceScreen(UIScreen* prevScreen)
{
    DVASSERT(renderTargetPrevScreen && renderTargetNextScreen);

    UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(prevScreen, renderTargetPrevScreen->GetTexture());
}

void UIScreenTransition::SetDestinationScreen(UIScreen* nextScreen)
{
    DVASSERT(renderTargetPrevScreen && renderTargetNextScreen);

    UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(nextScreen, renderTargetNextScreen->GetTexture());
}

void UIScreenTransition::EndTransition()
{
    ReleaseRenderTargets();
}

void UIScreenTransition::Update(float32 timeElapsed)
{
    UIScreen::Update(timeElapsed);

    currentTime += timeElapsed;
    normalizedTime = interpolationFunc(currentTime / duration);
    if (currentTime >= duration)
    {
        currentTime = duration;
        complete = true;
    }
}

void UIScreenTransition::Draw(const UIGeometricData& geometricData)
{
    if (renderTargetPrevScreen && renderTargetNextScreen)
    {
        Sprite::DrawState drawState;
        drawState.SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);

        drawState.SetScale(0.5f, 1.0f);
        drawState.SetPosition(0, 0);

        RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState, Color::White);

        drawState.SetScale(0.5f, 1.0f);
        drawState.SetPosition((VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dx) / 2.0f, 0);

        RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState, Color::White);
    }
}

void UIScreenTransition::SetDuration(float32 timeInSeconds)
{
    duration = timeInSeconds;
};

bool UIScreenTransition::IsComplete() const
{
    return complete;
}
};
