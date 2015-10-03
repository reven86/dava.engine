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

#include "Base/Platform.h"

#include "UI/UIMovieView.h"

#if defined(__DAVAENGINE_IPHONE__)
#   include "Platform/TemplateiOS/MovieViewControliOS.h"
#elif defined(__DAVAENGINE_MACOS__)
#   include "Platform/TemplateMacOS/MovieViewControlMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#   include "Platform/TemplateAndroid/MovieViewControlAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#   include "Platform/TemplateWin32/MovieViewControlWinUAP.h"
#else
// UIMovieView is not implemented for this platform yet, using stub one.
#   define DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW
#   include "Platform/MovieViewControlStub.h"
#   include "Render/RenderManager.h"
#   include "Render/RenderHelper.h"
#endif

namespace DAVA
{

UIMovieView::UIMovieView(const Rect &rect)
    : UIControl(rect)
    , movieViewControl(new MovieViewControl())
{
    movieViewControl->Initialize(rect);
}

UIMovieView::~UIMovieView()
{
    SafeDelete(movieViewControl);
}

void UIMovieView::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    movieViewControl->OpenMovie(moviePath, params);
}

void UIMovieView::SetPosition(const Vector2 &position)
{
    UIControl::SetPosition(position);

    Rect newRect = GetRect();
    movieViewControl->SetRect(newRect);
}

void UIMovieView::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);

    Rect newRect = GetRect();
    movieViewControl->SetRect(newRect);
}

void UIMovieView::Play()
{
    movieViewControl->Play();
}

void UIMovieView::Stop()
{
    movieViewControl->Stop();
}

void UIMovieView::Pause()
{
    movieViewControl->Pause();
}

void UIMovieView::Resume()
{
    movieViewControl->Resume();
}

bool UIMovieView::IsPlaying()
{
    return movieViewControl->IsPlaying();
}

void UIMovieView::SystemDraw(const UIGeometricData &geometricData)
{
    UIControl::SystemDraw(geometricData);

#if defined(DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW)
    Color curDebugDrawColor = GetDebugDrawColor();

    Rect absRect = GetAbsoluteRect();
    RenderManager::Instance()->SetColor(Color(1.0f, 0.4f, 0.8f, 1.0f));
    RenderHelper::Instance()->DrawRect(absRect, RenderState::RENDERSTATE_2D_BLEND);

    float32 minRadius = Min(GetSize().x, GetSize().y);
    RenderHelper::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 2, RenderState::RENDERSTATE_2D_BLEND);
    RenderHelper::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 3, RenderState::RENDERSTATE_2D_BLEND);
    RenderHelper::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 4, RenderState::RENDERSTATE_2D_BLEND);

    SetDebugDrawColor(curDebugDrawColor);
#endif
}

void UIMovieView::WillBecomeVisible()
{
    UIControl::WillBecomeVisible();
    movieViewControl->SetVisible(true);
}

void UIMovieView::WillBecomeInvisible()
{
    UIControl::WillBecomeInvisible();
    movieViewControl->SetVisible(false);
}

UIMovieView *UIMovieView::Clone()
{
    UIMovieView* uiMoviewView = new UIMovieView(GetRect());
    uiMoviewView->CopyDataFrom(this);
    return uiMoviewView;
}

}   // namespace DAVA
